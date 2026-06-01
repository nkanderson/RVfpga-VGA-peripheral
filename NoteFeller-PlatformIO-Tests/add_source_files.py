Import("env")
import os

shared_src = os.path.normpath(
    os.path.join(env.subst("$PROJECT_DIR"), "..", "..", "NoteFeller-App", "drivers")
)

env.Append(CPPPATH=[shared_src])
env.BuildSources(
    os.path.join(env.subst("$BUILD_DIR"), "drivers"),
    shared_src,
)
