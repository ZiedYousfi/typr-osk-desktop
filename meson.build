project(
  'typr-osk-desktop',
  'cpp',
  version: '0.0.1',
  default_options: [
    'cpp_std=c++20',
    'b_ndebug=true', # Disable debug assertions
    'optimization=3', # Full optimization for release
    'strip=true', # Strip symbols in the final binary
  ],
)

# Prefer shared Qt libraries on Windows to avoid issues with static
# Qt packages that may reference absolute object paths that are not
# available on CI (causing LNK1181: cannot open input file ...)
if host_machine.system() == 'windows'
  qt6_dep_shared = dependency('qt6', modules: ['Core', 'Gui', 'Widgets'], static: false, required: false)
  if qt6_dep_shared.found()
    qt6_dep = qt6_dep_shared
  else
    qt6_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets'])
  endif
else
  qt6_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets'])
endif

# Header files
headers = [
  'src/core/input.hpp',
  'src/core/layout.hpp',
  'src/backend/backend.hpp',
  'src/ui/widgets.hpp',
  'src/ui/window.hpp',
]

# Source files
sources = [
  'src/main.cpp',
  'src/core/input.cpp',
  'src/core/layout.cpp',
  'src/ui/window.cpp',
  'src/backend/key_utils.cpp',
]

# Include directories
inc_dirs = include_directories('src')

# Platform-specific sources and dependencies
deps = [qt6_dep]

if host_machine.system() == 'darwin'
  add_languages('objcpp', native: false)
  sources += 'src/backend/backend_macos.mm'
  sources += 'src/backend/output_listener_macos.mm'
  sources += 'src/ui/widgets_macos.mm'

  # macOS frameworks for input injection
  coregraphics_dep = dependency('appleframeworks', modules: ['CoreGraphics'])
  carbon_dep = dependency('appleframeworks', modules: ['Carbon'])
  appservices_dep = dependency('appleframeworks', modules: ['ApplicationServices'])
  foundation_dep = dependency('appleframeworks', modules: ['Foundation'])
  cocoa_dep = dependency('appleframeworks', modules: ['Cocoa'])

  deps += [coregraphics_dep, carbon_dep, appservices_dep, foundation_dep, cocoa_dep]
endif

if host_machine.system() == 'linux'
  sources += 'src/backend/backend_uinput.cpp'
  sources += 'src/backend/output_listener_x11.cpp'
  x11_dep = dependency('x11', required: false)
  xi_dep = dependency('xi', required: false)
  if x11_dep.found() and xi_dep.found()
    deps += [x11_dep, xi_dep]
  endif
endif

if host_machine.system() == 'windows'
  sources += 'src/backend/backend_windows.cpp'
  sources += 'src/backend/output_listener_windows.cpp'
  sources += 'src/ui/widgets_windows.cpp'
endif

executable(
  'typr-osk-desktop',
  sources + headers,
  dependencies: deps,
  include_directories: inc_dirs,
  install: false,
)
