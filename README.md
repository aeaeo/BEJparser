# About
Binary-encoded JSON (BEJ) parser to UTF-8 json format following the DMTF DSP0218 specification.

# Build
This following reference build is made on GNU/Linux Debian 13 Trixie.

Next packages are required: libgtest-dev googletest doxygen cmake

CMake is at least ver. 3.3.0

In terminal, go to the project root dir then execute the next following commands:
Let the clang be our compiler
<img width="1440" height="747" alt="image" src="https://github.com/user-attachments/assets/66c7443f-5514-4e58-b9dd-5c6c2ed2f007" />

# Example
Say we have a next .json file:
<img width="449" height="383" alt="image" src="https://github.com/user-attachments/assets/580e4189-0c01-498c-8c4f-d70401d3357d" />

Entry names are taken from this dictionary: https://www.dmtf.org/dsp/DSP8010 specific file 2025.3.zip/dictionaries/PCIeDevice_v1.bin

Encoding it with RDE-Dictionary tools (https://github.com/DMTF/RDE-Dictionary/):
<img width="1440" height="320" alt="image" src="https://github.com/user-attachments/assets/a7c5af83-8ecd-4485-ad09-4ad838996ce6" />

Decoding it with BEJparser and printing to stdout:
<img width="1421" height="402" alt="image" src="https://github.com/user-attachments/assets/f7e54c26-c1ba-4ce6-902d-964521c78cd2" />

.. or to the file:
<img width="1440" height="452" alt="image" src="https://github.com/user-attachments/assets/0831d78b-5249-4086-ac83-937a76be89f1" />
