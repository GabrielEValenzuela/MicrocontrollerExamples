SOURCE_FILES=$(find ./src -type f \( -name "*.cpp" -or -name "*.hpp" -or -name "*.h" -or -name "*.c" \) | tr "\n" " ")
SOURCE_FILES+=$(find ./include -type f \( -name "*.cpp" -or -name "*.hpp" -or -name "*.h" -or -name "*.c" \) | tr "\n" " ")

ERROR_FILE_FLAG=$PROJECT_PATH/clang_format_errors.txt

echo "Running: clang-format -i $SOURCE_FILES"

clang-format -i $SOURCE_FILES
