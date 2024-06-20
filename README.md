# Virtual Memory Manager Implementation

## Project Overview

This project involves the implementation of a Virtual Memory Manager as described in "Operating System Concepts" by Silberschatz, et al., 10th edition, on page P-51. The implementation follows specific modifications and requirements detailed below.

## Important Deadlines and Submission Guidelines

1. **Deadline**: Submissions after the specified deadline (8:00 am on 30.05) will not be accepted. Students with an official extension have until 8:00 am on 03.06.
2. **File Naming**:
    - **Implementation File**: The implementation file must be named `vm.c`.
    - **Makefile**: Ensure the Makefile is named exactly `Makefile`.
    - **Directory Naming**: Save both files in a directory named after the initials of your email in lowercase. For example, if your email is `gst@cesar.school`, the directory should be named `gst`.
    - **Archive**: The directory should be compressed into a `.tar` file named after the initials of your email. For example, `gst.tar`.
3. **Submission Method**: Submit the `.tar` file via the Google Classroom form for the activity.
4. **File Contents**: The directory should only contain `vm.c` and `Makefile`. Any additional files will result in automatic disqualification.
5. **Originality**: Any detected plagiarism will result in a zero grade.
6. **Compilation**: Submissions that fail to compile using the `make` command as specified will receive a zero grade.

## Implementation Details

### Virtual Memory Manager Requirements

- **Physical Memory**: Implement a system with only 128 frames in physical memory.
- **Programming Language**: The program must be implemented in C and executable on Linux systems.
- **Compilation**: Use `Makefile` to compile the program with the command `make`, resulting in an executable named `vm`.
- **Compiler**: The implementation will be validated using GCC 13.2.0, ensuring strict adherence to the specified output format and text.
- **Frame Management**: Fill frames from 0 to 127. When memory is full, use a page replacement algorithm to determine which frame to update.
- **Page Replacement Algorithms**: Implement FIFO and LRU page replacement algorithms. For the TLB, use FIFO.
- **Command Line Arguments**:
    - The first argument is a file containing logical addresses (similar to `address.txt` provided in the Classroom activity).
    - The second argument specifies the page replacement algorithm (`fifo` or `lru`).
    - Example: `./vm address.txt lru` indicates the use of the LRU page replacement algorithm.

### Output

- **Output File**: The output file should be named `correct.txt` and must follow the exact formatting of the provided example in the Classroom activity, which used the FIFO algorithm.

## Scoring

- **Test 1**: 20%
- **Test 2**: 20%
- **Test 3**: 30%
- **Test 4**: 30%

Ensure all implementation and submission guidelines are followed strictly to avoid disqualification. Good luck!
