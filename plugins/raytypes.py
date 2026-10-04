import pathlib

import cmy_reflector
from cmy_reflector import Macro, Plugin, Reflector, generate_reflection

# ----------------------------------------
# Plugin Configuration
# ----------------------------------------
PLUGIN_NAME = "raytypes"
PLUGIN_VERSION = "1.0.0"
PLUGIN_MAINTAINERS = ["oonamo"]
PLUGIN_DESCRIPTION = "Adds RayLib types to cmyreflection"

# Helpers for user's and authors to determine whether plugin is available
PLUGIN_DEFINE_MACRO = f"CMY_HAS_{PLUGIN_NAME.upper()}_PLUGIN"
PLUGIN_ENABLED_MACRO = f"CMY_PLUGIN_{PLUGIN_NAME.upper()}_ENABLED"

# Instantiate plugin
plugin = Plugin(
    name=PLUGIN_NAME,
    version=PLUGIN_VERSION,
    maintainers=PLUGIN_MAINTAINERS,
    description=PLUGIN_DESCRIPTION,
    includes=["<stdbool.h>"],
    macros=[
        Macro.define(PLUGIN_DEFINE_MACRO, "1", f"{PLUGIN_NAME} plugin is available"),
        Macro.default(PLUGIN_ENABLED_MACRO, "1", f"Enables the {PLUGIN_NAME} plugin"),
        # Add Custom Macros Here
        # Macro.raw, Macro.include, Macro.default, Macro.define,
    ],
)


# ----------------------------------------
# Setup & Extensions
# ----------------------------------------
@plugin.setup
def setup(reflector: Reflector):
    """Registers custom field extensions that will be added to the C extension structs"""

    plugin_dir = pathlib.Path(__file__).parent
    stub_file = plugin_dir / "raylib_stubs.h"

    with open(stub_file, "r") as f:
        c_code = f.read()

    generate_reflection(reflector, "raylib_stubs.h", c_code)


# ----------------------------------------
# Registration
# ----------------------------------------
cmy_reflector.add_plugin(plugin)
