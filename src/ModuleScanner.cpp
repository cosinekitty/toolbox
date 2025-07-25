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
            json_t* root = json_object();

        public:
            explicit ModuleDatabase(const std::string& _filename)
                : filename(_filename)
                {}

            virtual ~ModuleDatabase()
            {
                unload();
            }

            void unload()
            {
                if (root)
                {
                    json_decref(root);
                    root = nullptr;
                }
            }

            void load()
            {
                unload();

                const char *fn = filename.c_str();
                if (FILE* infile = fopen(fn, "rt"))
                {
                    json_error_t error;
                    root = json_loadf(infile, 0, &error);
                    if (root)
                        INFO("Loaded json from file file: %s", fn);
                    else
                        WARN("Error at line %d, column %d in file %s: %s", error.line, error.column, fn, error.text);
                    fclose(infile);
                }

                if (root == nullptr)
                    root = json_object();
            }

            void save()
            {
                const char *fn = filename.c_str();
                if (json_is_object(root))
                {
                    FILE* outfile = fopen(fn, "wt");
                    if (outfile)
                    {
                        json_dumpf(root, outfile, JSON_INDENT(4));
                        fclose(outfile);
                    }
                    else
                    {
                        INFO("Could not open file for write: %s", fn);
                    }
                }
                else
                {
                    WARN("Root object is not valid. Skipping save to file %s", fn);
                }
            }

            void update(ModuleWidget* widget)
            {
                if (!widget || !widget->model || !widget->model->plugin)
                    return;

                const Plugin* plugin = widget->model->plugin;

                // root[pluginSlug][moduleSlug].modules.append(moduleRecord)
                // Get or create the plugin object.
                json_t* pluginObj = json_object_get(root, plugin->slug.c_str());
                if (!pluginObj)
                {
                    pluginObj = json_object();
                    json_object_set_new(root, plugin->slug.c_str(), pluginObj);
                }
                json_object_set_new(pluginObj, "version",     json_string(plugin->version.c_str()));
                json_object_set_new(pluginObj, "license",     json_string(plugin->license.c_str()));
                json_object_set_new(pluginObj, "name",        json_string(plugin->name.c_str()));
                json_object_set_new(pluginObj, "author",      json_string(plugin->author.c_str()));
                json_object_set_new(pluginObj, "authorEmail", json_string(plugin->authorEmail.c_str()));

                // Ensure there is an array called "modules".
                json_t* modulesArray = json_object_get(pluginObj, "modules");
                if (!modulesArray)
                {
                    modulesArray = json_array();
                    json_object_set_new(pluginObj, "modules", modulesArray);
                }

                // Search for a module with the same slug.
                // If it exists, replace it. Otherwise, create a new entry.
                json_t* moduleJson = serializeModule(widget);
                size_t index;
                json_t *existingModule{};
                const char* moduleSlug = widget->model->slug.c_str();
                bool found = false;
                json_array_foreach(modulesArray, index, existingModule)
                {
                    json_t *slug = json_object_get(existingModule, "slug");
                    if (json_is_string(slug) && 0==strcmp(json_string_value(slug), moduleSlug))
                    {
                        json_array_set_new(modulesArray, index, moduleJson);
                        found = true;
                        break;
                    }
                }

                if (!found)
                    json_array_append_new(modulesArray, moduleJson);
            }

            json_t* serializeModule(ModuleWidget* widget)
            {
                if (widget && widget->model)
                {
                    json_t* root = json_object();
                    json_object_set_new(root, "slug", json_string(widget->model->slug.c_str()));
                    json_object_set_new(root, "name", json_string(widget->model->name.c_str()));
                    addParams(root, widget);
                    addPorts(root, "inputs",  widget->getInputs());
                    addPorts(root, "outputs", widget->getOutputs());
                    return root;
                }
                return nullptr;
            }

            void addParams(json_t* root, ModuleWidget* widget)
            {
                json_t* params = json_array();
                for (ParamWidget* paramWidget : widget->getParams())
                    if (json_t* paramJson = serializeParam(paramWidget))
                        json_array_append_new(params, paramJson);
                json_object_set_new(root, "params", params);
            }

            void addPorts(json_t* root, const char *key, std::vector<PortWidget*> portList)
            {
                json_t* ports = json_array();
                for (PortWidget* p : portList)
                {
                    PortInfo *info = p->getPortInfo();
                    json_t* portJson = json_object();
                    populateWidget(portJson, p);
                    json_object_set_new(portJson, "portId", json_integer(info->portId));
                    json_object_set_new(portJson, "name", json_string(info->getFullName().c_str()));
                    json_object_set_new(portJson, "description", json_string(info->getDescription().c_str()));
                    json_array_append_new(ports, portJson);
                }
                json_object_set_new(root, key, ports);
            }

            json_t* serializeParam(ParamWidget* paramWidget)
            {
                if (ParamQuantity* qty = paramWidget->getParamQuantity())
                {
                    json_t* paramJson = json_object();
                    populateWidget(paramJson, paramWidget);
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

            void populateWidget(json_t* root, Widget* widget)
            {
                json_t* box = json_object();
                json_object_set_new(root, "box", box);
                addVec(box, "pos",  widget->box.pos);
                addVec(box, "size", widget->box.size);
            }

            void addVec(json_t* root, const char* key, const Vec& vec)
            {
                json_t* obj = json_object();
                json_object_set_new(obj, "x", json_real(vec.x));
                json_object_set_new(obj, "y", json_real(vec.y));
                json_object_set_new(root, key, obj);
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
                    if (auto mw = dynamic_cast<ModuleWidget*>(w))
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
