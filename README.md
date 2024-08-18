# Lime Window Manager

> **NOTE:** This project is still in development and is not ready for use.

The Lime Window Manager is a window manager for X11, seamlessly blending the 
best of both tiled and floating window management. Designed with simplicity and 
elegance in mind, it aims to provide a clean and uncluttered interface while 
still delivering all the essential functionalities that users expect from a 
modern window manager. It is an ideal choice for users who seek a balance 
between aesthetics and productivity.

## Building

To build this project locally, you will need the following dependencies:

> **NOTE:** The following command is intended for Debian based systems.

```bash
sudo apt install \
    gcc \
    make \
    libx11-dev \
    libcairo2-dev 
```

Once the dependencies are installed, you can build the project by running:

```bash
make
```

This will compile the source code and generate an executable in the `./bin`
directory.

## License

The Lime Window Manager is licensed under the GPL-3.0 License. This license 
reflects our commitment to ensuring that this software remains free and
open-source.  We believe in the values of freedom, transparency, and
collaboration that the GPL-3.0 promotes, allowing users to freely use, modify,
and distribute the software, ensuring that it remains a community-driven project.

For more details, see the `LICENSE.md` file.
