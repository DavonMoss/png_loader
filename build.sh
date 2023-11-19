echo "Building..."

EXECUTABLE_NAME="loadpng"

rm $EXECUTABLE_NAME
g++ src/*.cpp -o $EXECUTABLE_NAME
