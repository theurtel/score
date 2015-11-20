#pragma once
#include <QObject>
#include <iscore/plugins/qt_interfaces/GUIApplicationContextPlugin_QtInterface.hpp>
#include <iscore/plugins/qt_interfaces/FactoryInterface_QtInterface.hpp>
#include <iscore/plugins/qt_interfaces/PluginRequirements_QtInterface.hpp>

class iscore_plugin_ossia final :
    public QObject,
    public iscore::GUIApplicationContextPlugin_QtInterface,
    public iscore::FactoryInterface_QtInterface,
        public iscore::PluginRequirementslInterface_QtInterface
{
        Q_OBJECT
        Q_PLUGIN_METADATA(IID GUIApplicationContextPlugin_QtInterface_iid)
        Q_INTERFACES(
            iscore::GUIApplicationContextPlugin_QtInterface
            iscore::FactoryInterface_QtInterface
                iscore::PluginRequirementslInterface_QtInterface
        )

    public:
        iscore_plugin_ossia();
        virtual ~iscore_plugin_ossia() = default;

        iscore::GUIApplicationContextPlugin* make_applicationPlugin(
                iscore::Application& app) override;

        // Contains the OSC, MIDI, Minuit factories
        std::vector<iscore::FactoryInterfaceBase*> factories(
                const iscore::ApplicationContext&,
                const iscore::FactoryBaseKey& factoryName) const override;

        QStringList required() const override;
};

