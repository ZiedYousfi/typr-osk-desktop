project(
  'typr-osk-desktop',
  'cpp',
  'objcpp',
  version: '0.0.1',
  default_options: [
    'cpp_std=c++20',
  ],
)

qt6_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets'])

# Header files
headers = [
  'src/core/input.hpp',
  'src/input/input.hpp',
  'src/ui/ui.hpp',
]

# Source files
sources = [
  'src/main.cpp',
  'src/core/input.cpp',
]

# Include directories
inc_dirs = include_directories('src')

# Platform-specific sources and dependencies
deps = [qt6_dep]

if host_machine.system() == 'darwin'
  sources += 'src/input/input_macos.mm'
  sources += 'src/ui/ui_macos.mm'

  # macOS frameworks for input injection
  coregraphics_dep = dependency('appleframeworks', modules: ['CoreGraphics'])
  carbon_dep = dependency('appleframeworks', modules: ['Carbon'])
  appservices_dep = dependency('appleframeworks', modules: ['ApplicationServices'])
  foundation_dep = dependency('appleframeworks', modules: ['Foundation'])
  cocoa_dep = dependency('appleframeworks', modules: ['Cocoa'])

  deps += [coregraphics_dep, carbon_dep, appservices_dep, foundation_dep, cocoa_dep]
endif

executable(
  'typr-osk-desktop',
  sources + headers,
  dependencies: deps,
  include_directories: inc_dirs,
  install: false,
)
