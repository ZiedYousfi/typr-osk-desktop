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

# Source files
sources = [
  'src/main.cpp',
]

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

  deps += [coregraphics_dep, carbon_dep, appservices_dep, foundation_dep]
endif

executable(
  'typr-osk-desktop',
  sources,
  dependencies: deps,
  install: false,
)
