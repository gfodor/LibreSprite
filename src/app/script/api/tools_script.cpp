
#include "she/pointer_type.h"
#include "script/engine.h"
#include "app/app.h"
#include "app/pref/preferences.h"
#include "app/tools/tool.h"
#include "app/tools/tool_box.h"
#include "app/tools/freehand_algorithm.h"

using namespace she;
using namespace app;

class ToolsScriptObject : public script::ScriptObject {
public:
  ToolsScriptObject() {

    addFunction("getBrushSize",
        [] (std::string id) -> int { 
          auto tool = App::instance()->toolBox()->getToolById(id);
          if (tool == nullptr) return -1;
          return (int)Preferences::instance().tool(tool).brush.size();
        })
      .doc("Gets the brush size for a known tool");

    addFunction("setBrushSize",
        [] (std::string id, int size) -> int {
          auto tool = App::instance()->toolBox()->getToolById(id);
          if (tool == nullptr) return -1;
          return Preferences::instance().tool(tool).brush.size(size);
        })
      .doc("Set the brush size for a known tool");

    addFunction("getFreehandAlgorithm",
      [] (std::string id) -> std::string { 
        auto tool = App::instance()->toolBox()->getToolById(id);
        if (tool == nullptr) return "";

        switch ((tools::FreehandAlgorithm)Preferences::instance().tool(tool).freehandAlgorithm()) {
          case tools::FreehandAlgorithm::PIXEL_PERFECT:
            return "pixel-perfect";
          case tools::FreehandAlgorithm::REGULAR:
            return "regular";
          case tools::FreehandAlgorithm::DOTS:
            return "dots";
          default:
            return "default";
        }
      })
    .doc("Gets the freehand algorithm for a known tool");

    addFunction("setFreehandAlgorithm",
      [] (std::string id, std::string algorithm) -> std::string {
        auto tool = App::instance()->toolBox()->getToolById(id);
        if (tool == nullptr) return "";
        Preferences::instance().tool(tool).freehandAlgorithm(algorithm == "pixel-perfect" ? tools::FreehandAlgorithm::PIXEL_PERFECT :
                                                           algorithm == "regular" ? tools::FreehandAlgorithm::REGULAR :
                                                           algorithm == "dots" ? tools::FreehandAlgorithm::DOTS :
                                                           tools::FreehandAlgorithm::DEFAULT);
        return algorithm;
      })
      .doc("Set the freehand algorithm for a known tool");

    addFunction("setFgColorRgb",
      [] (int r, int g, int b) -> bool {
        Preferences::instance().colorBar.fgColor(app::Color::fromRgb(r, g, b));
        return true;
      })
      .doc("Set the foreground color via RGB");

    addFunction("setFgColorHsv",
      [] (int h, int s, int v) -> bool {
        Preferences::instance().colorBar.fgColor(app::Color::fromHsv(h, s, v));
        return true;
      })
      .doc("Set the foreground color via HSV");

    addFunction("setFgColorGray",
      [] (int g) -> bool {
        Preferences::instance().colorBar.fgColor(app::Color::fromGray(g));
        return true;
      })
      .doc("Set the foreground color via Gray");

    addFunction("setFgColorIndex",
      [] (int index) -> bool {
        Preferences::instance().colorBar.fgColor(app::Color::fromIndex(index));
        return true;
      })
      .doc("Set the foreground color via Index");

    addFunction("setBgColorRgb",
      [] (int r, int g, int b) -> bool {
        Preferences::instance().colorBar.bgColor(app::Color::fromRgb(r, g, b));
        return true;
      })
      .doc("Set the background color via RGB");

    addFunction("setBgColorHsv",
      [] (int h, int s, int v) -> bool {
        Preferences::instance().colorBar.bgColor(app::Color::fromHsv(h, s, v));
        return true;
      })
      .doc("Set the background color via HSV");

    addFunction("setBgColorGray",
      [] (int g) -> bool {
        Preferences::instance().colorBar.bgColor(app::Color::fromGray(g));
        return true;
      })
      .doc("Set the background color via Gray");

    addFunction("setBgColorIndex",
      [] (int index) -> bool {
        Preferences::instance().colorBar.bgColor(app::Color::fromIndex(index));
        return true;
      })
      .doc("Set the background color via Index");

    makeGlobal("tools");
  }
};

static script::ScriptObject::Regular<ToolsScriptObject> reg("ToolsScriptObject", {"global"});
