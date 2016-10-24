#pragma once
#include <Explorer/Common/AddressSettings/Widgets/AddressSettingsWidget.hpp>
#include <Device/Address/AddressSettings.hpp>
#include <State/Widgets/Values/VecWidgets.hpp>
#include <State/Widgets/Values/NumericValueWidget.hpp>
#include <QComboBox>
#include <QFormLayout>
namespace Explorer
{
template<typename T>
class AddressArraySettingsWidget final : public AddressSettingsWidget
{
    public:
        explicit AddressArraySettingsWidget(QWidget* parent = nullptr)
            : AddressSettingsWidget(parent)
        {
            // TODO for each value, display the corresopnding settings widget (with min/max/etc...).
            // In order to do this properly,
            // we have to separate the global and per-tuple settings widgets...
        }

        Device::AddressSettings getSettings() const override
        {
            auto settings = getCommonSettings();
            settings.value.val = T{};
            return settings;
        }

        void setSettings(const Device::AddressSettings& settings) override
        {
            setCommonSettings(settings);
        }

        Device::AddressSettings getDefaultSettings() const override
        {
            return {};
        }
};
template<std::size_t N>
class AddressVecSettingsWidget final : public AddressSettingsWidget
{
    public:
        explicit AddressVecSettingsWidget(QWidget* parent = nullptr)
            : AddressSettingsWidget(parent)
        {
            m_valueEdit = new State::VecWidget<N>{this};
            m_domainSelector = new QComboBox{this};
            m_domainSelector->insertItems(tr("Float"), tr("Vec"));
            m_domainSelector->setCurrentIndex(0);
            connect(m_domainSelector, SignalUtils::QComboBox_currentIndexChanged_int(),
                    this, &AddressVecSettingsWidget<N>::on_domainTypeChange);

            m_domainFloatEdit = new State::NumericDomainWidget<float>{this};
            m_domainVecEdit = new State::VecDomainWidget<N>{this};

            m_layout->insertRow(0, tr("Value"), m_valueEdit);
            m_layout->insertRow(1, tr("Domain Type"), m_domainSelector);
            m_layout->insertRow(2, tr("Domain"), m_domainFloatEdit);
        }

        Device::AddressSettings getSettings() const override
        {
            auto settings = getCommonSettings();
            settings.value.val = m_valueEdit->value();
            if(m_domainFloatEdit->isVisible())
                settings.domain = m_domainFloatEdit->domain();
            else
                settings.domain = m_domainVecEdit->domain();
            return settings;
        }

        void setSettings(const Device::AddressSettings& settings) override
        {
            setCommonSettings(settings);
            m_valueEdit->setValue(State::value::convert<std::array<float, N>>(settings.value));

        }

        Device::AddressSettings getDefaultSettings() const override
        {
            Device::AddressSettings s;
            s.value.val = std::array<float, N>{};
            s.domain = ossia::net::make_domain(ossia::Float{0}, ossia::Float{1});
            return {};
        }

        void on_domainTypeChange(int id)
        {
            switch(id)
            {
                // Float
                case 0:
                {
                    m_domainVecEdit->setHidden(true);
                    m_domainFloatEdit->setHidden(false);
                    m_layout->replaceWidget(m_domainVecEdit, m_domainFloatEdit, Qt::FindDirectChildrenOnly);
                    break;
                }
                    // Vec
                case 1:
                {
                    m_domainVecEdit->setHidden(false);
                    m_domainFloatEdit->setHidden(true);
                    m_layout->replaceWidget(m_domainFloatEdit, m_domainVecEdit, Qt::FindDirectChildrenOnly);
                    break;
                }
            }
        }


        State::VecWidget<N>* m_valueEdit{};
        QComboBox* m_domainSelector{};
        State::NumericValueWidget<float>* m_domainFloatEdit{};
        State::VecDomainWidget<N>* m_domainVecEdit{};
};
}
