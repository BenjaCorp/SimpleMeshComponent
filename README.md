# SimpleMeshComponent

## Overview

The `SimpleMeshComponent` provides a simplified alternative to Unreal Engine's `ProceduralMeshComponent`. Designed for developers seeking a straightforward starting point for custom mesh components, it focuses on a lightweight implementation, leveraging a custom mesh proxy for optimized performance. This component is ideal for projects that require basic mesh manipulation functionalities with reduced code complexity and memory footprint.

## Features

- **Custom Mesh Proxy**: Utilizes a lightweight proxy for efficient mesh processing.
- **Dynamic Mesh Creation**: Supports the dynamic creation of mesh sections.
- **Performance Optimized**: Designed for reduced code complexity and memory usage.
- **Ease of Use**: Provides a straightforward API for basic mesh operations, making it ideal for custom extensions.


## Getting Started

![SimpleMeshDemo GIF](https://github.com/BenjaCorp/SimpleMeshComponent/blob/main/SimpleMeshSubdivise.gif?raw=true)
![Blueprint Nodes](https://github.com/BenjaCorp/SimpleMeshComponent/assets/19375442/c503c868-4ec0-480f-909f-9d58bc1c0b28)

### Installation

To integrate the `SimpleMeshComponent` into your Unreal Engine project:
- Place the folder "SimpleMeshComponent" on your Project/Plugins folder. 
- Add 'SimpleMeshComponent' on Build.CS Right clik on your .Uproject -> Generate
- Launch your .sln, and Compile the project on IDE.

## Usage

To use the `SimpleMeshComponent`, you will typically follow these steps:

1. **Include the Component**: Include `SimpleMeshComponent.h` in your actor or object.
2. **Create a Mesh Section**: Call `CreateMeshSection()` to dynamically create mesh sections.
3. **Update Mesh Section**: Use `UpdateMeshSection()` to update mesh sections with new vertices and indices.
4. **Remove Mesh Section**: Call `RemoveMeshSection()` to delete sections of the mesh.

## Code Structure

- `SimpleMeshComponent.h/cpp`: Defines the main component functionality, including mesh section creation, update, and removal.
- `SimpleMeshProxy.h`: Implements the custom proxy for efficient mesh handling.

## Contributing

Contributions are welcome to extend and improve `SimpleMeshComponent`. If you have suggestions or improvements, please feel free to fork the repository, make your changes, and submit a pull request.

## License

This project is licensed under the MIT License - Proxy by : @StevenChristy - Component by: BenjaCorp at IolaCorpStudio
