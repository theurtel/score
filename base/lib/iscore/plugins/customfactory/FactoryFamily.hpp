#pragma once
#include <iscore/plugins/customfactory/FactoryInterface.hpp>
#include <iscore/plugins/customfactory/FactoryMap.hpp>
#include <iscore/tools/ForEachType.hpp>
#include <iscore/tools/std/Pointer.hpp>
#include <iscore/tools/std/Algorithms.hpp>
#include <iscore/tools/std/IndirectContainer.hpp>
#include <iscore/tools/Todo.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/iterator/indirect_iterator.hpp>

#include <iscore_lib_base_export.h>
#include <QMetaType>

namespace bmi = boost::multi_index;

namespace iscore
{
/**
 * @brief The FactoryFamily class
 *
 * Keeps the factories, so that they can be found easily.
 */
class ISCORE_LIB_BASE_EXPORT FactoryListInterface
{
    public:
        static constexpr bool factory_list_tag = true;
        FactoryListInterface() = default;
        FactoryListInterface(const FactoryListInterface&) = delete;
        FactoryListInterface& operator=(const FactoryListInterface&) = delete;
        virtual ~FactoryListInterface();

        // Example : InspectorWidgetFactory
        virtual iscore::AbstractFactoryKey abstractFactoryKey() const = 0;

        // This function is called whenever a new factory interface
        // is added to this family.
        virtual void insert(std::unique_ptr<iscore::FactoryInterfaceBase>) = 0;
};

// FIXME They should take an export macro also ?
template<typename FactoryType>
class ConcreteFactoryList :
        public iscore::FactoryListInterface,
        public IndirectMap<
                bmi::multi_index_container<
                    std::unique_ptr<FactoryType>,
                    bmi::indexed_by<
                        bmi::hashed_unique<
                            bmi::const_mem_fun<
                                iscore::GenericFactoryInterface<typename FactoryType::ConcreteFactoryKey>,
                                add_cref_t<typename FactoryType::ConcreteFactoryKey>,
                                &FactoryType::template key<typename FactoryType::ConcreteFactoryKey>
                            >
                        >
                    >
                >
        >
{
    public:
        using factory_type = FactoryType;
        using key_type = typename FactoryType::ConcreteFactoryKey;

        ~ConcreteFactoryList() noexcept override = default;

        static const iscore::AbstractFactoryKey& static_abstractFactoryKey() {
            return FactoryType::static_abstractFactoryKey();
        }

        iscore::AbstractFactoryKey abstractFactoryKey() const final override {
            return FactoryType::static_abstractFactoryKey();
        }

        void insert(std::unique_ptr<iscore::FactoryInterfaceBase> e) final override
        {
			auto pf = dynamic_unique_ptr_cast<factory_type>(std::move(e));
            if(pf)
            {
                auto it = this->map.find(pf->template key<key_type>());
                if(it == this->map.end())
                {
                    this->map.insert(std::move(pf));
                }
                else
                {
                    ISCORE_ABORT;
                }
            }
        }

        auto get(const key_type& k) const
        {
            auto it = this->map.find(k);
            return (it != this->map.end()) ? it->get() : nullptr;
        }

    protected:
        const auto& list() const
        { return this->map; }
};

template<typename T>
class MatchingFactory : public iscore::ConcreteFactoryList<T>
{
    public:
        template<typename Fun, typename... Args>
        auto make(Fun f, Args&&... args) const
        {
            auto it = find_if(
                          *this,
                          [&] (const auto& elt)
            { return elt.matches(std::forward<Args>(args)...); });

            return (it != this->end())
                    ? ((*it).*f)(std::forward<Args>(args)...)
                    : decltype(((*it).*f)(std::forward<Args>(args)...)){};
        }
};

}


template<typename Base_T,
         typename... Args>
struct GenericFactoryInserter
{
        std::vector<std::unique_ptr<Base_T>> vec;
        GenericFactoryInserter()
        {
            vec.reserve(sizeof...(Args));
            for_each_type<TypeList<Args...>>(*this);
        }

        template<typename TheClass>
        void operator()()
        {
            vec.push_back(std::make_unique<TheClass>());
        }
};

template<typename... Args>
auto make_ptr_vector()
{
    return GenericFactoryInserter<Args...>{}.vec;
}

/**
 * @brief FactoryBuilder
 *
 * This class allows the user to customize the
 * creation of the factory by specializing it with the actual
 * factory type. An example is in iscore_plugin_scenario.cpp.
 */
template<typename Context_T,
         typename Factory_T>
struct FactoryBuilder // sorry padre for I have sinned
{
        static auto make(const Context_T&)
        {
            return std::make_unique<Factory_T>();
        }
};

template<typename Context_T,
         typename Base_T,
         typename... Args>
struct ContextualGenericFactoryInserter
{
        const Context_T& context;
        std::vector<std::unique_ptr<Base_T>> vec;
        ContextualGenericFactoryInserter(const Context_T& ctx):
            context{ctx}
        {
            vec.reserve(sizeof...(Args));
            for_each_type<TypeList<Args...>>(*this);
        }

        template<typename TheClass>
        void operator()()
        {
            vec.push_back(FactoryBuilder<Context_T, TheClass>::make(context));
        }
};


template<typename Context_T, typename Base_T, typename... Args>
auto make_ptr_vector(const Context_T& context)
{
    return ContextualGenericFactoryInserter<Context_T, Base_T, Args...>{context}.vec;
}

#define ISCORE_STANDARD_FACTORY(FactorizedElementName) \
class FactorizedElementName ## FactoryTag {}; \
using FactorizedElementName ## FactoryKey = StringKey<FactorizedElementName ## FactoryTag>; \
Q_DECLARE_METATYPE(FactorizedElementName ## FactoryKey) \
\
class FactorizedElementName ## Factory : \
        public iscore::GenericFactoryInterface<FactorizedElementName ## FactoryKey> \
{ \
        ISCORE_ABSTRACT_FACTORY_DECL(#FactorizedElementName) \
    public: \
            using ConcreteFactoryKey = FactorizedElementName ## FactoryKey; \
        virtual std::unique_ptr<FactorizedElementName> make() = 0; \
}; \
class FactorizedElementName ## FactoryList final : public iscore::FactoryListInterface \
{ \
       ISCORE_FACTORY_LIST_DECL(FactorizedElementName ## Factory) \
};

