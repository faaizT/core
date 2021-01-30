from distutils.core import setup, Extension

extension_mod = Extension("_reward", ["_reward_module.cc", "RewardFromAntibiotics.cpp"], extra_link_args=["-stdlib=libc++", "-mmacosx-version-min=10.15"])

setup(name = "reward", ext_modules=[extension_mod])