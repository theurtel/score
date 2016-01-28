#include <iscore/serialization/DataStreamVisitor.hpp>
#include <iscore/serialization/JSONVisitor.hpp>
#include <iscore/tools/VariantSerialization.hpp>
#include <QJsonObject>
#include <QJsonValue>

#include "Expression.hpp"
#include "Relation.hpp"
#include <iscore/serialization/JSONValueVisitor.hpp>

template <typename T> class Reader;
template <typename T> class TypeToName;
template <typename T> class Writer;

template<>
void Visitor<Reader<DataStream>>::readFrom(const State::Relation& rel)
{
    m_stream << rel.lhs
             << rel.op
             << rel.rhs;

    insertDelimiter();
}

template<>
void Visitor<Reader<JSONObject>>::readFrom(const State::Relation& rel)
{
    // TODO harmonize from... with marshall(..) in VisitorCommon.hpp
    m_obj["LHS"] = toJsonObject(rel.lhs);
    m_obj["Op"] = toJsonValue(rel.op);
    m_obj["RHS"] = toJsonObject(rel.rhs);
}

template<>
void Visitor<Writer<DataStream>>::writeTo(State::Relation& rel)
{
    m_stream >> rel.lhs
             >> rel.op
             >> rel.rhs;

    checkDelimiter();
}

template<>
void Visitor<Writer<JSONObject>>::writeTo(State::Relation& rel)
{
    fromJsonObject(m_obj["LHS"].toObject(), rel.lhs);
    fromJsonObject(m_obj["RHS"].toObject(), rel.rhs);
    fromJsonValue(m_obj["Op"], rel.op);
}

template<> class TypeToName<State::Address>
{ public: static constexpr const char * name() { return "Address"; } };

template<> class TypeToName<State::Value>
{ public: static constexpr const char * name() { return "Value"; } };

template<> class TypeToName<State::Relation>
{ public: static constexpr const char * name() { return "Relation"; } };

template<> class TypeToName<State::BinaryOperator>
{ public: static constexpr const char * name() { return "BinOp"; } };

template<> class TypeToName<State::UnaryOperator>
{ public: static constexpr const char * name() { return "UnOp"; } };


template<>
ISCORE_LIB_STATE_EXPORT void Visitor<Reader<DataStream>>::readFrom(const ExprData& expr)
{
    readFrom(expr.m_data);
    insertDelimiter();
}

template<>
ISCORE_LIB_STATE_EXPORT void Visitor<Reader<JSONObject>>::readFrom(const ExprData& expr)
{
    readFrom(expr.m_data);
}

template<>
ISCORE_LIB_STATE_EXPORT void Visitor<Writer<DataStream>>::writeTo(ExprData& expr)
{
    writeTo(expr.m_data);
    checkDelimiter();
}

template<>
ISCORE_LIB_STATE_EXPORT void Visitor<Writer<JSONObject>>::writeTo(ExprData& expr)
{
    writeTo(expr.m_data);
}


