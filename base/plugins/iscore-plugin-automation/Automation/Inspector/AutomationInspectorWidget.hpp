#pragma once
#include <Process/Inspector/ProcessInspectorWidgetDelegate.hpp>
#include <Automation/AutomationModel.hpp>
#include <QString>
#include <iscore/command/Dispatchers/CommandDispatcher.hpp>

class QWidget;

namespace iscore{
class Document;
struct DocumentContext;
struct Address;
}
class AddressEditWidget;
class DeviceExplorerModel;
class QDoubleSpinBox;

namespace Automation
{
class ProcessModel;
class InspectorWidget final :
        public ProcessInspectorWidgetDelegate_T<Automation::ProcessModel>
{
    public:
        explicit InspectorWidget(
                const ProcessModel& object,
                const iscore::DocumentContext& context,
                QWidget* parent);

    public slots:
        void on_addressChange(const iscore::Address& newText);
        void on_minValueChanged();
        void on_maxValueChanged();

    private:
        AddressEditWidget* m_lineEdit{};
        QDoubleSpinBox* m_minsb{}, *m_maxsb{};

        CommandDispatcher<> m_dispatcher;
};
}
