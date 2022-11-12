import generator

project = generator.Project([ generator.Target("vulkan-scene", [ "src/main.c" ]) ])
project.generate("cl", "link")