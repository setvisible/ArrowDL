#
# Return version value of Boost (ex: "1.79.0" for version 107900), or 0 if error
#
# USAGE:
#   BOOST_VERSION_WITH_DOTS = $$getBoostVersionWithDots($$BOOST_VERSION)
#
defineReplace(getBoostVersionWithDots) {
    BOOST_VERSION = $$1

    Boost_VERSION_MAJOR = $${BOOST_VERSION}
    Boost_VERSION_MINOR = $${BOOST_VERSION}
    Boost_VERSION_PATCH = $${BOOST_VERSION}

    Boost_VERSION_MAJOR ~= s/^(.*).....$/\1/
    Boost_VERSION_MINOR ~= s/^.*(..)..$/\1/
    Boost_VERSION_PATCH ~= s/^.*(..)$/\1/

    greaterThan(QT_MAJOR_VERSION, 5) {
        greaterThan(QT_MINOR_VERSION, 8) {
            # num_add() was introduced in Qt 5.8
            # trick to replace '00' by '0'
            Boost_VERSION_MAJOR = $$num_add($${Boost_VERSION_MAJOR}, 0)
            Boost_VERSION_MINOR = $$num_add($${Boost_VERSION_MINOR}, 0)
            Boost_VERSION_PATCH = $$num_add($${Boost_VERSION_PATCH}, 0)
        }
    }

    ret=$${Boost_VERSION_MAJOR}.$${Boost_VERSION_MINOR}.$${Boost_VERSION_PATCH}

    return($${ret})
}
