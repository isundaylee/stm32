# HACKS EVERYWHERE!
# IT'S A SANDBOX AFTER ALL

add_executable(sigrok_sandbox
    main.cpp
    )

set_target_properties(sigrok_sandbox
    PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )

# -Wl,-framework -Wl,CoreFoundation -lsigc-2.0

target_link_directories(sigrok_sandbox
    PRIVATE
        /Users/jiahaoli/.local/lib
        /usr/local/Cellar/glibmm/2.58.0/lib
        /usr/local/Cellar/glib/2.58.3/lib
        /usr/local/opt/gettext/lib
        /usr/local/Cellar/libsigc++/2.10.1/lib
    )

target_link_libraries(sigrok_sandbox
    PRIVATE
        sigrokcxx
        sigrok
        glibmm-2.4
        gobject-2.0
        glib-2.0
        intl
    )

target_include_directories(sigrok_sandbox
    PRIVATE
        /Users/jiahaoli/.local/include
        /usr/local/Cellar/libftdi/1.4/include/libftdi1
        /usr/local/Cellar/libusb/1.0.22/include/libusb-1.0
        /usr/local/Cellar/libzip/1.5.1/include
        /usr/local/Cellar/glibmm/2.58.0/include/glibmm-2.4
        /usr/local/Cellar/glibmm/2.58.0/lib/glibmm-2.4/include
        /usr/local/Cellar/glib/2.58.3/include/glib-2.0
        /usr/local/Cellar/glib/2.58.3/lib/glib-2.0/include
        /usr/local/opt/gettext/include
        /usr/local/Cellar/pcre/8.42/include
        /usr/local/Cellar/libsigc++/2.10.1/include/sigc++-2.0
        /usr/local/Cellar/libsigc++/2.10.1/lib/sigc++-2.0/include
    )
