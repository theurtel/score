#pragma once

#include <score/statemachine/GraphicsSceneToolPalette.hpp>
#include <Scenario/Document/DisplayedElements/DisplayedElementsPresenter.hpp>

namespace Library
{
struct ProcessData;
}
namespace Scenario
{
class ScenarioDocumentPresenter;
class CentralIntervalDisplay
{
public:
  explicit CentralIntervalDisplay(ScenarioDocumentPresenter& p);
  ~CentralIntervalDisplay();

  // Init method necessary because of
  // https://stackoverflow.com/questions/69050714/observing-the-state-of-a-variant-during-construction
  void init();

  void on_addProcessFromLibrary(const Library::ProcessData& dat);
  void on_visibleRectChanged(const QRectF&);
  void on_executionTimer();

  ScenarioDocumentPresenter& parent;
  DisplayedElementsPresenter presenter;

private:
  std::unique_ptr<GraphicsSceneToolPalette> m_stateMachine;
};
}
