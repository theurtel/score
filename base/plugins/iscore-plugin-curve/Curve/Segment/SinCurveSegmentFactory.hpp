#pragma once
#include "CurveSegmentFactory.hpp"

class SinCurveSegmentFactory : public CurveSegmentFactory
{
    public:
        QString name() const;

        CurveSegmentModel *make(
                const id_type<CurveSegmentModel>& id,
                QObject* parent) override;

        CurveSegmentModel *load(
                const VisitorVariant& vis,
                QObject* parent) override;
};
