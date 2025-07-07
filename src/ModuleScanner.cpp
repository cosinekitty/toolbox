#include "toolbox_vcvrack.hpp"

namespace Toolbox
{
    namespace ModuleScanner
    {
        enum ParamId
        {
            SAVE_BUTTON_PARAM,
            PARAMS_LEN
        };

        enum InputId
        {
            INPUTS_LEN
        };

        enum OutputId
        {
            OUTPUTS_LEN
        };

        enum LightId
        {
            SAVE_BUTTON_LIGHT,
            LIGHTS_LEN
        };


        class ModuleDatabase
        {
        private:
            const std::string filename;
            json_t* root = nullptr;

        public:
            explicit ModuleDatabase(const std::string& _filename)
                : filename(_filename)
                {}

            virtual ~ModuleDatabase()
            {
                if (root)
                {
                    json_decref(root);
                    root = nullptr;
                }
            }

            void load()
            {
                json_decref(root);

                FILE* infile = fopen(filename.c_str(), "rt");
                if (infile)
                {
                    json_error_t error;
                    root = json_loadf(infile, 0, &error);
                    fclose(infile);
                }
                else
                {
                    INFO("Could not open file for read: %s", filename.c_str());
                }

                if (root == nullptr)
                    root = json_object();
            }

            void save()
            {
                FILE* outfile = fopen(filename.c_str(), "wt");
                if (outfile)
                {
                    json_dumpf(root, outfile, JSON_INDENT(4));
                    fclose(outfile);
                }
                else
                {
                    INFO("Could not open file for write: %s", filename.c_str());
                }
            }

            void update(ModuleWidget* widget)
            {
                if (!widget || !widget->model || !widget->model->plugin)
                    return;

                const std::string& moduleSlug = widget->model->slug;
                const std::string& pluginSlug = widget->model->plugin->slug;

                json_t* moduleJson = serializeModule(widget);

                // root[pluginSlug][moduleSlug] = moduleRecord
                // Get or create the plugin object.
                json_t* pluginObj = json_object_get(root, pluginSlug.c_str());
                if (!pluginObj)
                {
                    pluginObj = json_object();
                    json_object_set_new(root, pluginSlug.c_str(), pluginObj);
                }

                // Set or update the module entry.
                json_object_set(pluginObj, moduleSlug.c_str(), moduleJson);
            }

            json_t* serializeModule(ModuleWidget* widget)
            {
                json_t* root = json_object();
                json_t* params = json_array();
                json_object_set_new(root, "params", params);
                for (ParamWidget* paramWidget : widget->getParams())
                    if (json_t* paramJson = serializeParam(paramWidget))
                        json_array_append_new(params, paramJson);
                return root;
            }

            json_t* serializeParam(ParamWidget* paramWidget)
            {
                if (ParamQuantity* qty = paramWidget->getParamQuantity())
                {
                    json_t* paramJson = json_object();
                    json_object_set_new(paramJson, "paramId", json_integer(qty->paramId));
                    json_object_set_new(paramJson, "name", json_string(qty->name.c_str()));
                    json_object_set_new(paramJson, "description", json_string(qty->description.c_str()));
                    json_object_set_new(paramJson, "unit", json_string(qty->unit.c_str()));
                    json_object_set_new(paramJson, "minValue", json_real(qty->minValue));
                    json_object_set_new(paramJson, "maxValue", json_real(qty->maxValue));
                    json_object_set_new(paramJson, "defaultValue", json_real(qty->defaultValue));
                    json_object_set_new(paramJson, "displayBase", json_real(qty->displayBase));
                    json_object_set_new(paramJson, "displayMultiplier", json_real(qty->displayMultiplier));
                    json_object_set_new(paramJson, "displayOffset", json_real(qty->displayOffset));
                    return paramJson;
                }
                return nullptr;
            }
        };


        struct ModuleScannerModule : ToolboxModule
        {
            GateTriggerReceiver saveReceiver;

            explicit ModuleScannerModule()
            {
                config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
                configButton(SAVE_BUTTON_PARAM, "Update module database");
                initialize();
            }

            void initialize()
            {
                saveReceiver.initialize();
            }

            void onReset(const ResetEvent& e) override
            {
                initialize();
            }

            void process(const ProcessArgs& args) override
            {
                if (saveReceiver.updateTrigger(10 * params.at(SAVE_BUTTON_PARAM).getValue()))
                    updateModuleDatabase(asset::user("ModuleScanner.json"));
            }

            void updateModuleDatabase(const std::string& dataFileName)
            {
                INFO("Beginning scan.");
                ModuleDatabase db(dataFileName);
                db.load();
                for (Widget* w : APP->scene->rack->getModuleContainer()->children)
                    if (auto mw = dynamic_cast<ModuleWidget*>(w); mw && mw->module)
                        db.update(mw);
                db.save();
            }
        };


        struct ModuleScannerWidget : ToolboxWidget
        {
            explicit ModuleScannerWidget(ModuleScannerModule* module)
            {
                setModule(module);
                const std::string svgFileName = asset::plugin(pluginInstance, "res/modscan.svg");
                setPanel(new SvgOverlay(window::Svg::load(svgFileName)));
                addParam(createLightParamCentered<ToolboxButton>(
                    mm2px(Vec((8/2) * HP_WIDTH_MM, 20.0)),
                    module,
                    SAVE_BUTTON_PARAM,
                    SAVE_BUTTON_LIGHT
                ));
            }
        };
    }
}

Model* modelToolboxModuleScanner = createModel<Toolbox::ModuleScanner::ModuleScannerModule, Toolbox::ModuleScanner::ModuleScannerWidget>("ModuleScanner");
