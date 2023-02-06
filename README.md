# libeeprom - a simple and generic EEPROM read and write library

This library is used to read and write EEPROM information. It works for
any EEPROM, but has special support for Compulab SoMs by being aware of the EEPROM
layout, and using that information to display the EEPROM contents in a human
readable format.

The utility provides a WYSIWYG UI for updating EEPROM contents; the user can
specify the target field by name, and provide the new value in the same
formatting as displayed by the `read` command. The user can also clear a field
by name. Direct offset/value mode of writing is also supported.

Finally, the utility can list accessible devices on i2c buses, or a more
concise listing of just the EEPROM devices, depending on the available
interfaces in the OS.

Example run:
----
# ./eeprom-util read 2 0x50
Major Revision                1.00
Minor Revision                0.00
1st MAC Address               00:01:c0:13:91:d0
2nd MAC Address               00:01:ca:12:91:d0
Production Date               29/Aug/2013
Serial Number                 ffffffffffff123456789abc
3rd MAC Address (WIFI)        ff:ff:ff:ff:ff:ff
4th MAC Address (Bluetooth)   ff:ff:ff:ff:ff:ff
Layout Version                02
Reserved fields               (83 bytes)
Product Name                  CM-FX6
Product Options #1            C1200QM-D2G-N0-
Product Options #2            ND0-U5-E-A-WB
Product Options #3
Reserved fields               (64 bytes)

# ./eeprom-util write fields 2 0x50 "Production Date=01/Jun/2014"
# ./eeprom-util clear fields 2 0x50 "Serial Number" "Minor Revision"
----

= Requirements

The utility requires a Linux system with either /dev/i2c interface, or a loaded
EEPROM driver (at24).

= Building

To build the utility you can type one of the following:

* `make` - Only read functionality will be supported, system default linking type.
* `make write` - Enable the write capabilities, system default linking type.
* `make static` - Only read functionality, force static linking of libraries.
* `make write_static` - Enable the write capabilities, force static linking of libraries.
* `make debug` - Enable the write capabilities, compile with debug flags.

= Adding custom EEPROM layouts

To add support for your custom EEPROM layouts do the following:

*layout.c*:

* Create a new `struct field` array with the fields of your layout.
+
Each field must define a name, a short name, a size (in bytes) and a type.
+
The available field's types are listed under `enum field_type` in `field.h`.
+
The total size of all fields must not exceed the defined `EEPROM_SIZE` in
`layout.h`
* Update `detect_layout()` function to properly auto-detect your layout.
+
Notice the value of `LAYOUT_CHECK_BYTE`. It is the offset of the "layout
version" field.
* Update `build_layout()` function to select the fields array of your layout.

*layout.h*:

* Add the version or the name of your layout to `enum layout_version`

*parser.c*:

* (optional) Update `parse_layout_version()` to enable manual selection of your
layout using the `-l <layout_version>` command line option.

WARNING: Some features of the utility may not work correctly if your custom layout
does not retain backward compatibility with existing Compulab layouts. To
ensure full support, your custom layout should expand existing layouts only
within the reserved fields area. In any case, try not to change the offset of
the "Layout Version" field to avoid complicating the auto detection of your
layout by the utility.
