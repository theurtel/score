// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <Scenario/Document/ScenarioDocument/ScenarioDocumentModel.hpp>

#include "BaseScenarioComponent.hpp"
#include "DocumentPlugin.hpp"
#include <Engine/Executor/IntervalComponent.hpp>
#include <Engine/Executor/StateProcessComponent.hpp>
#include <Engine/Executor/Settings/ExecutorModel.hpp>
#include <Explorer/DocumentPlugin/DeviceDocumentPlugin.hpp>
#include <Scenario/Document/BaseScenario/BaseScenario.hpp>
#include <core/document/Document.hpp>
#include <core/document/DocumentModel.hpp>
#include <score/plugins/documentdelegate/plugin/DocumentPlugin.hpp>

#include <Engine/Protocols/Audio/AudioDevice.hpp>
#include <Engine/ApplicationPlugin.hpp>
#include <ossia/dataflow/audio_protocol.hpp>
#include <ossia/editor/scenario/time_interval.hpp>
#include <ossia/dataflow/port.hpp>
#include <score/actions/ActionManager.hpp>
#include <Engine/score2OSSIA.hpp>
#include <Scenario/Application/ScenarioActions.hpp>
#include <ossia/dataflow/graph.hpp>
namespace Engine
{
namespace Execution
{
DocumentPlugin::DocumentPlugin(
    const score::DocumentContext& ctx,
    Id<score::DocumentPlugin> id,
    QObject* parent)
    : score::DocumentPlugin{ctx, std::move(id),
                             "OSSIADocumentPlugin", parent}
    , audioproto{new ossia::audio_protocol}
    , audio_dev{std::unique_ptr<ossia::net::protocol_base>(audioproto), "audio"}
    , midi_dev{std::make_unique<ossia::net::multiplex_protocol>(), "midi"}
    , m_ctx{
          ctx, m_base,
          ctx.plugin<Explorer::DeviceDocumentPlugin>(),
          ctx.app.interfaces<ProcessComponentFactoryList>(),
          ctx.app.interfaces<StateProcessComponentFactoryList>(),
          {}, {},
          m_editionQueue, *this
      }
    , m_base{m_ctx, this}
{
  midi_ins.push_back(ossia::net::create_parameter<ossia::midi_generic_parameter>(midi_dev.get_root_node(), "/0/in"));

  midi_outs.push_back(ossia::net::create_parameter<ossia::midi_generic_parameter>(midi_dev.get_root_node(), "/0/out"));

  execGraph = std::make_shared<ossia::graph>();
  //audioproto->reload();
  audio_device = new Dataflow::AudioDevice(
    {Dataflow::AudioProtocolFactory::static_concreteKey(), "audio", {}},
    audio_dev);

  ctx.plugin<Explorer::DeviceDocumentPlugin>().list().setAudioDevice(audio_device);

  auto& model = ctx.model<Scenario::ScenarioDocumentModel>();
  model.cables.mutable_added.connect<DocumentPlugin, &DocumentPlugin::on_cableCreated>(*this);
  model.cables.removing.connect<DocumentPlugin, &DocumentPlugin::on_cableRemoved>(*this);

  con(m_base, &Engine::Execution::BaseScenarioElement::finished, this,
      [=] {
        auto& stop_action = context().doc.app.actions.action<Actions::Stop>();
        stop_action.action()->trigger();
      }, Qt::QueuedConnection);

  connect(this, &DocumentPlugin::finished,
          this, &DocumentPlugin::on_finished, Qt::QueuedConnection);
}

void DocumentPlugin::on_cableCreated(Process::Cable& c)
{
  connectCable(c);
}

void DocumentPlugin::on_cableRemoved(const Process::Cable& c)
{
  if(!m_base.active())
    return;
  auto cable = c.exec;
  auto graph = execGraph;

  context().executionQueue.enqueue([cable,graph] {
    graph->disconnect(cable);
  });
}

void DocumentPlugin::connectCable(Process::Cable& cable)
{
  if(!m_base.active())
    return;
  if(auto port_src = cable.source().try_find(context().doc))
  {
    auto it = outlets.find(port_src);
    if(it != outlets.end()) {
      cable.source_node = it->second.first;
      cable.source_port = it->second.second;
    }
  }
  if(auto port_snk = cable.sink().try_find(context().doc))
  {
    auto it = inlets.find(port_snk);
    if(it != inlets.end()){
      cable.sink_node = it->second.first;
      cable.sink_port = it->second.second;
    }
  }

  if(cable.source_node && cable.sink_node)
  {
    context().executionQueue.enqueue(
          [type=cable.type()
          ,outlet=cable.source_node
          ,inlet=cable.sink_node
          ,outport=cable.source_port
          ,inport=cable.sink_port
          ,graph=execGraph
          ]
    {
      ossia::edge_ptr edge;
      switch(type)
      {
        case Process::CableType::ImmediateStrict:
        {
          edge = ossia::make_edge(
                   ossia::immediate_strict_connection{},
                   outport, inport, outlet, inlet);
          break;
        }
        case Process::CableType::ImmediateGlutton:
        {
          edge = ossia::make_edge(
                   ossia::immediate_glutton_connection{},
                   outport, inport, outlet, inlet);
          break;
        }
        case Process::CableType::DelayedStrict:
        {
          edge = ossia::make_edge(
                   ossia::delayed_strict_connection{},
                   outport, inport, outlet, inlet);
          break;
        }
        case Process::CableType::DelayedGlutton:
        {
          edge = ossia::make_edge(
                   ossia::delayed_glutton_connection{},
                   outport, inport, outlet, inlet);
          break;
        }
      }

      graph->connect(edge);
    });
  }
}


DocumentPlugin::~DocumentPlugin()
{
  if (m_base.active())
  {
    m_base.baseInterval().stop();
    clear();
  }
  audioproto->stop();
}

void DocumentPlugin::on_finished()
{
  auto& doc = context().doc.model<Scenario::ScenarioDocumentModel>();

  runAllCommands();
  m_base.cleanup();
  runAllCommands();

  for(Process::Cable& cbl : doc.cables)
  {
    cbl.source_node.reset();
    cbl.sink_node.reset();
    cbl.source_port.reset();
    cbl.sink_port.reset();
    cbl.exec.reset();
  }

  inlets.clear();
  outlets.clear();
  m_cables.clear();
  execGraph->clear();
  execGraph.reset();


  for (Process::Cable& cable : doc.cables)
  {
    if(cable.source_node)
    {
      cable.source_node->clear();
      cable.source_node.reset();
    }
    if(cable.sink_node)
    {
      cable.sink_node->clear();
      cable.sink_node.reset();
    }

    if(cable.exec)
    {
      cable.exec->clear();
      cable.exec.reset();
    }
  }

  execState.clear();
  execState.globalState.clear();
  execState.messages.clear();
  execState.mess_values.clear();

  for(auto& v : runtime_connections)
  {
    for(auto& con : v.second)
    {
      QObject::disconnect(con);
    }
  }
  runtime_connections.clear();

  execGraph = std::make_shared<ossia::graph>();
}
void DocumentPlugin::reload(Scenario::IntervalModel& cst)
{
  if (m_base.active())
  {
    m_base.baseInterval().stop();
  }
  clear();
  auto& ctx = score::DocumentPlugin::context();
  auto& settings = ctx.app.settings<Engine::Execution::Settings::Model>();
  m_ctx.time = settings.makeTimeFunction(ctx);
  m_ctx.reverseTime = settings.makeReverseTimeFunction(ctx);
  auto parent = dynamic_cast<Scenario::ScenarioInterface*>(cst.parent());
  SCORE_ASSERT(parent);
  m_base.init(BaseScenarioRefContainer{cst, *parent});

  auto& model = context().doc.model<Scenario::ScenarioDocumentModel>();
  //qDebug() << context().document.findChildren<Process::Node*>();
  for(auto& cable : model.cables)
  {
    connectCable(cable);
  }

  runAllCommands();
}

void DocumentPlugin::clear()
{
  if(m_base.active())
  {
    runAllCommands();
    m_base.cleanup();
    runAllCommands();
    execGraph.reset();
    execGraph = std::make_shared<ossia::graph>();
  }
}

void DocumentPlugin::on_documentClosing()
{
  if (m_base.active())
  {
    m_base.baseInterval().stop();
    m_ctx.context().doc.app.guiApplicationPlugin<Engine::ApplicationPlugin>().on_stop();
    clear();
  }
}

const BaseScenarioElement& DocumentPlugin::baseScenario() const
{
  return m_base;
}

bool DocumentPlugin::isPlaying() const
{
  return m_base.active();
}

void DocumentPlugin::runAllCommands() const
{
  bool ok = false;
  ExecutionCommand com;
  do {
    ok = m_editionQueue.try_dequeue(com);
    if(ok && com)
      com();
  } while(ok);
}


void DocumentPlugin::register_node(
    const Process::ProcessModel& proc,
    const std::shared_ptr<ossia::graph_node>& node)
{
  register_node(proc.inlets(), proc.outlets(), node);
}
void DocumentPlugin::unregister_node(
    const Process::ProcessModel& proc,
    const std::shared_ptr<ossia::graph_node>& node)
{
  unregister_node(proc.inlets(), proc.outlets(), node);
}

void DocumentPlugin::set_destination(
    const State::AddressAccessor& address,
    const ossia::inlet_ptr& port)
{
  if(auto ossia_addr = Engine::score_to_ossia::findAddress(context().devices.list(), address.address))
  {
    auto p = ossia_addr->get_parameter();
    if(p)
    {
      auto& qual = address.qualifiers.get();

      m_editionQueue.enqueue([=] {
        port->address = p;
        if(ossia::value_port* dat = port->data.target<ossia::value_port>()) {
          if(qual.unit)
            dat->type = qual.unit;
          dat->index = qual.accessors;
        }
      });
    }
    else
    {
      m_editionQueue.enqueue([=] {
        port->address = ossia_addr;
      });
    }
  }
  else
  {
    m_editionQueue.enqueue([=] {
      port->address = {};
      if(ossia::value_port* dat = port->data.target<ossia::value_port>()) {
        dat->type = {};
        dat->index.clear();
      }
    });
  }
}
void DocumentPlugin::set_destination(
    const State::AddressAccessor& address,
    const ossia::outlet_ptr& port)
{
  if(auto ossia_addr = Engine::score_to_ossia::findAddress(context().devices.list(), address.address))
  {
    auto p = ossia_addr->get_parameter();
    if(p)
    {
      auto& qual = address.qualifiers.get();

      m_editionQueue.enqueue([=] {
        port->address = p;
        if(ossia::value_port* dat = port->data.target<ossia::value_port>()) {
          if(qual.unit)
            dat->type = qual.unit;
          dat->index = qual.accessors;
        }
      });
    }
    else
    {
      m_editionQueue.enqueue([=] {
        port->address = ossia_addr;
      });
    }
  }
  else
  {
    m_editionQueue.enqueue([=] {
      port->address = {};
      if(ossia::value_port* dat = port->data.target<ossia::value_port>()) {
        dat->type = {};
        dat->index.clear();
      }
    });
  }
}

void DocumentPlugin::register_node(
    const Process::Inlets& proc_inlets, const Process::Outlets& proc_outlets,
    const std::shared_ptr<ossia::graph_node>& node)
{
  if(node)
  {
    const std::size_t n_inlets = proc_inlets.size();
    const std::size_t n_outlets = proc_outlets.size();

    SCORE_ASSERT(node->inputs().size() >= n_inlets);
    SCORE_ASSERT(node->outputs().size() >= n_outlets);

    auto& runtime_connection = runtime_connections[node];

    runtime_connection.reserve(runtime_connection.size() + n_inlets + n_outlets);
    for(std::size_t i = 0; i < n_inlets; i++)
    {
      runtime_connection.push_back(connect(proc_inlets[i], &Process::Port::addressChanged,
              this, [this,port=node->inputs()[i]] (const State::AddressAccessor& address) {
        set_destination(address, port);
      }));
      SCORE_ASSERT(node->inputs()[i]);
      set_destination(proc_inlets[i]->address(), node->inputs()[i]);

      inlets.insert({ proc_inlets[i], std::make_pair( node, node->inputs()[i] ) });
    }

    for(std::size_t i = 0; i < n_outlets; i++)
    {
      runtime_connection.push_back(connect(proc_outlets[i], &Process::Port::addressChanged,
              this, [this,port=node->outputs()[i]] (const State::AddressAccessor& address) {
        set_destination(address, port);
      }));
      SCORE_ASSERT(node->outputs()[i]);
      set_destination(proc_outlets[i]->address(), node->outputs()[i]);

      outlets.insert({ proc_outlets[i], std::make_pair( node, node->outputs()[i] ) });
    }

    m_editionQueue.enqueue([=] {
      execGraph->add_node(node);
    });
  }
}
void DocumentPlugin::unregister_node(
    const Process::Inlets& proc_inlets, const Process::Outlets& proc_outlets,
    const std::shared_ptr<ossia::graph_node>& node)
{
  if(node)
  {
    m_editionQueue.enqueue([=] {
      node->clear();
      execGraph->remove_node(node);
    });

    for(const auto& con : runtime_connections[node])
    {
      QObject::disconnect(con);
    }
    runtime_connections.erase(node);
  }

  for(auto ptr : proc_inlets)
    inlets.erase(ptr);
  for(auto ptr : proc_outlets)
    outlets.erase(ptr);
}

void DocumentPlugin::unregister_node_soft(
    const Process::Inlets& proc_inlets, const Process::Outlets& proc_outlets,
    const std::shared_ptr<ossia::graph_node>& node)
{
  if(node)
  {
    for(const auto& con : runtime_connections[node])
    {
      QObject::disconnect(con);
    }
    runtime_connections.erase(node);
  }

  for(auto ptr : proc_inlets)
    inlets.erase(ptr);
  for(auto ptr : proc_outlets)
    outlets.erase(ptr);
}
}
}
