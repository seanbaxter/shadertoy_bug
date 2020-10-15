# Fragment shader SPIR-V module contamination

```
# Link the individual fragment shaders into linked.spv
$ spirv-link modulation.spv bands.spv -o linked.spv

# Compile executables that read from the non-linked modules
$ clang++ shadertoy.cxx -DMODULATION -o modulation -lGL -lgl3w -lglfw
$ clang++ shadertoy.cxx -DBANDS -o bands -lGL -lgl3w -lglfw

# Compile executables that read from the linked module.
$ clang++ shadertoy.cxx -DLINKED -DMODULATION -o modulation_linked -lGL -lgl3w -lglfw
$ clang++ shadertoy.cxx -DLINKED -DBANDS -o bands_linked -lGL -lgl3w -lglfw
```

The modulation shader is corrupted when loaded from the linked.spv module. The bands shader isn't corrupted this time, but I also witnessed two-way corruption while preparing this bug report.

[./modulation](modulation.png)
[./modulation_linked](modulation_linked.png0
[./bands](bands.png))