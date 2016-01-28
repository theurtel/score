#pragma once

/*
This file is used to define simple data structure to simplify the code when needed
*/

#include <Process/TimeValue.hpp>
#include <iscore/tools/ModelPath.hpp>
#include <iscore/tools/SettableIdentifier.hpp>
#include <QByteArray>
#include <QMap>
#include <QPair>

namespace Scenario
{
class ConstraintModel;
class ConstraintViewModel;
class RackModel;
class TimeNodeModel;

struct TimenodeProperties {
    TimeValue oldDate;
    TimeValue newDate;
};

struct ConstraintProperties {
    TimeValue oldMin;
    TimeValue newMin;
    TimeValue oldMax;
    TimeValue newMax;
    QPair<
        QPair<
            Path<ConstraintModel>,
            QByteArray
        >, // The constraint data
        QMap< // Mapping for the view models of this constraint
            Id<ConstraintViewModel>,
            Id<RackModel>
        >
     > savedDisplay;
};

struct ElementsProperties {
    QMap<Id<TimeNodeModel>, TimenodeProperties> timenodes;
    QMap<Id<ConstraintModel>, ConstraintProperties> constraints;
};
}
