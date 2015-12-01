#pragma once
#include <Process/ModelMetadata.hpp>
#include <Process/Process.hpp>
#include <Process/TimeValue.hpp>
#include <Scenario/Document/Constraint/ConstraintDurations.hpp>
#include <Scenario/Document/Constraint/Rack/RackModel.hpp>
#include <Scenario/Document/ModelConsistency.hpp>
#include <boost/optional/optional.hpp>
#include <iscore/plugins/documentdelegate/plugin/ElementPluginModelList.hpp>
#include <iscore/selection/Selectable.hpp>
#include <QObject>
#include <nano_signal_slot.hpp>

#include <QString>
#include <QVector>

#include <iscore/tools/IdentifiedObject.hpp>
#include <iscore/tools/NotifyingMap.hpp>
#include <iscore/tools/SettableIdentifier.hpp>

class DataStream;
class JSONObject;
class StateModel;

namespace OSSIA
{
}

class ConstraintViewModel;
class FullViewConstraintViewModel;

class ConstraintModel final : public IdentifiedObject<ConstraintModel>, public Nano::Observer
{
        Q_OBJECT
        ISCORE_METADATA("ConstraintModel")

        ISCORE_SERIALIZE_FRIENDS(ConstraintModel, DataStream)
        ISCORE_SERIALIZE_FRIENDS(ConstraintModel, JSONObject)

        // TODO must go in view model
        Q_PROPERTY(double heightPercentage
                   READ heightPercentage
                   WRITE setHeightPercentage
                   NOTIFY heightPercentageChanged)

    public:
        /** Properties of the class **/
        Selectable selection;
        ModelMetadata metadata;
        ModelConsistency consistency;
        ConstraintDurations duration{*this};

        iscore::ElementPluginModelList pluginModelList;

        static QString description()
        { return QObject::tr("Constraint"); }

        /** The class **/
        ConstraintModel(const Id<ConstraintModel>&,
                        const Id<ConstraintViewModel>& fullViewId,
                        double yPos,
                        QObject* parent);


        // Copy
        ConstraintModel(const ConstraintModel &source,
                        const Id<ConstraintModel>& id,
                        QObject* parent);

        // Serialization
        template<typename Deserializer>
        ConstraintModel(Deserializer&& vis, QObject* parent) :
            IdentifiedObject{vis, parent}
        {
            initConnections();
            vis.writeTo(*this);
        }

        // Factories for the view models.
        template<typename ViewModelType> // Arg might be an id or a datastream [
        ViewModelType* makeConstraintViewModel(
                const Id<ConstraintViewModel>& id,
                QObject* parent)
        {
            auto viewmodel = new ViewModelType {id, *this, parent};
            setupConstraintViewModel(viewmodel);
            return viewmodel;
        }

        // Note : the Constraint does not have ownership (it's generally the Slot)
        void setupConstraintViewModel(ConstraintViewModel* viewmodel);

        const Id<StateModel>& startState() const;
        void setStartState(const Id<StateModel>& eventId);

        const Id<StateModel>& endState() const;
        void setEndState(const Id<StateModel> &endState);

        // Here we won't remove / add things from the outside so it is safe to
        // return a reference
        const QVector<ConstraintViewModel*>& viewModels() const
        { return m_constraintViewModels; }

        const TimeValue& startDate() const;
        void setStartDate(const TimeValue& start);
        void translate(const TimeValue& deltaTime);

        double heightPercentage() const;


        FullViewConstraintViewModel* fullView() const
        {
            return m_fullViewModel;
        }

        void setFullView(FullViewConstraintViewModel* fv);



        void startExecution();
        void stopExecution();
        // Resets the execution display recursively
        void reset();

        NotifyingMap<RackModel> racks;
        NotifyingMap<Process> processes;

        bool looping() const;
        void setLooping(bool looping);

    signals:
        void viewModelCreated(const ConstraintViewModel&);
        void viewModelRemoved(const QObject*);

        void heightPercentageChanged(double);

        void startDateChanged(const TimeValue&);

        void focusChanged(bool);
        void loopingChanged(bool);

    public slots:
        void setHeightPercentage(double arg);

    private slots:
        void on_destroyedViewModel(QObject*);

    private:
        void initConnections();
        void on_rackAdded(const RackModel& rack);

        // The small view constraint view models that show this constraint
        // The constraint does not have ownership of these: their parent (in the Qt sense) are
        // the scenario view models
        QVector<ConstraintViewModel*> m_constraintViewModels;

        // Model for the full view.
        // Note : it is also present in m_constraintViewModels.
        FullViewConstraintViewModel* m_fullViewModel {};

        Id<StateModel> m_startState;
        Id<StateModel> m_endState;

        TimeValue m_startDate; // origin

        double m_heightPercentage {0.5};

        bool m_looping{false};
};
