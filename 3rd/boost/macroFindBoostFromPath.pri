#
# Try to find the Boost directory from the PATH environment variable
#
# USAGE:
#   MY_VARIABLE = $$findBoostFromPath()
#
defineReplace(findBoostFromPath) {
    headers =
    USER_ENVIRONMENT_PATH = $$getenv(PATH)
    ITEMS = $$split(USER_ENVIRONMENT_PATH, ;)
    for(ITEM, ITEMS){
        exists( $$system_path( $$ITEM/boost/version.hpp ) ) {
            ITEM = $$clean_path( $$ITEM )
            # message(Found Boost at $$ITEM)
            headers = $$ITEM
            return($$headers)
        }
    }
}
