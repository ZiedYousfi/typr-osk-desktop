project(
  'typr-osk-desktop',
  'cpp',
  version: '0.0.1',
  default_options: [
    'cpp_std=c++20',
  ],
)

qt6_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets'])

executable(
  'typr-osk-desktop',
  'src/main.cpp',
  dependencies: [qt6_dep],
  install: false,
)