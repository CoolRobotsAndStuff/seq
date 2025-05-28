mkdir -p $(dirname "$0")/build/examples
gcc -o $(dirname "$0")/build/$1 $(dirname "$0")/$1.c

if [ $? -eq 0 ]; then
    if [ "$2" = "run" ]; then
        $(dirname "$0")/build/$1
    fi
fi

# if [ "$1" = "example" ]; then
# elif [ "$1" = "probe" ]; then
#     gcc -o build/probe probe.c
# elif [ "$1" = "" ]; then
#     echo No target provided
# else
#     echo "Invalid target $1"
# fi
