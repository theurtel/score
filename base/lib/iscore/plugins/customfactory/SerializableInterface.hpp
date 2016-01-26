#pragma once
#include <iscore/plugins/customfactory/UuidKey.hpp>

#include <iscore/serialization/JSONVisitor.hpp>

namespace iscore
{
template<typename T>
class ISCORE_LIB_BASE_EXPORT SerializableInterface
{
    public:
        SerializableInterface() = default;
        virtual ~SerializableInterface() = default;
        virtual UuidKey<T> uuid() const = 0;
        void serialize(const VisitorVariant& vis) const
        {
            switch(vis.identifier)
            {
                case DataStream::type():
                {
                    auto& v = static_cast<DataStream::Serializer&>(vis.visitor);
                    v.readFrom(uuid().impl());
                    break;
                }
                case JSONObject::type():
                {
                    auto& v = static_cast<JSONObject::Serializer&>(vis.visitor);
                    v.m_obj["uuid"] = toJsonValue(uuid().impl());
                    break;
                }
                default:
                    ISCORE_ABORT;
            }

            serialize_impl(vis);
        }

    protected:
        virtual void serialize_impl(const VisitorVariant& vis) const
        {

        }
};
}

template<typename Type>
UuidKey<Type> deserialize_key(Deserializer<JSONObject>& des)
{
    return fromJsonValue<boost::uuids::uuid>(des.m_obj["uuid"]);;

}

template<typename Type>
UuidKey<Type> deserialize_key(Deserializer<DataStream>& des)
{
    boost::uuids::uuid uid;
    des.writeTo(uid);
    return uid;

}


template<typename FactoryList_T, typename Deserializer, typename... Args>
auto deserialize_interface(
        const FactoryList_T& factories,
        Deserializer& des,
        Args&&... args)
{
    // Deserialize the interface identifier
    auto k = deserialize_key<typename FactoryList_T::factory_type>(des);

    // Get the factory
    auto concrete_factory = factories.get(k);
    if(!concrete_factory)
        return nullptr;

    // Create the object
    return concrete_factory->load(des.toVariant(), std::forward<Args>(args)...);
}


