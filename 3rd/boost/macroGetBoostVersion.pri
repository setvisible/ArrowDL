#
# Return version value of Boost (ex: "107200" for version 1.72.0), or 0 if error
#
# USAGE:
#   MY_VARIABLE = $$getBoostVersion($$PATH)
#
defineReplace(getBoostVersion) {
    boostversionfile = $$1
    boostversion =
    ITEMS = $$cat($${boostversionfile}, lines)
    keyword_to_find = $$quote($${LITERAL_HASH}define BOOST_VERSION .*)
    keyword_to_replace = $$quote($${LITERAL_HASH}define BOOST_VERSION)
    for(ITEM, ITEMS){
        contains(ITEM, $${keyword_to_find}) {
            BOOST_VERSION = $$replace(ITEM, $${keyword_to_replace}, "")
            BOOST_VERSION = $$replace(BOOST_VERSION, \\s, "") # trim spaces
            # message(Found Boost version: $${BOOST_VERSION})
            boostversion = $${BOOST_VERSION}
            return($${boostversion})
        }
    }  
    # message(Boost version not found)
    boostversion = 0
    return($${boostversion})
}
