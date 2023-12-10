# Seminal Input Features Detection Using LLVM


## Goals

The primary goal of this project is to develop a static analysis tool using LLVM that identifies seminal input features in C programs. These features are parts of the input that determine the behavior of the program at key points. 

## Design Idea Pathway

Our approach to solving the seminal input feature detection problem in C programs involves implementing a sophisticated def-use chain that can effectively trace the calling relationships within the program. The core of our design idea involves the following steps:

1. **Implementing a Robust Def-Use Chain**: Develop a mechanism to trace the definition and usage chain of variables throughout the program. This tracking is crucial for understanding how different variables influence each other and the overall program flow.The transfer relationship between variables is stored in an adjacency list constructed using a hashmap.

2. **Identifying Loop Termination Variables**: Within loops, our analysis focuses on pinpointing the specific variables that determine the loop's termination. This is done by analyzing loop conditions and the variables that affect these conditions.

3. **Tracing Calling Relationships in Global Functions**: Examine the global scope to find the calling relationships between user input variables and other variables. This step is vital to establish a connection between user inputs and their influence on the program.

4. **Matching IO Variables with Loop Termination Variables**: Utilize a hashmap to match which IO variable controls the terminating loop variable. This matching is the key to linking user inputs with their direct impact on critical program execution points.

5. **Outputting Target Variables and Line Numbers**: The final step is to output the identified seminal input features, specifically highlighting the target variable and its line number in the source code. This output is formatted in a clear and structured manner for ease of analysis.


## Challenges

1. **Def-Use Relation Complexity**: Tracing def-use relations in a program can be intricate, especially in the presence of loops and branching.
2. **Deeper Semantic Analysis**: Some cases require understanding the semantics of I/O and other APIs, which goes beyond simple def-use relations.
3. **Diverse Program Structures**: Programs may have varied structures and logic, making a one-size-fits-all approach ineffective.
4. **Performance Overhead**: Ensuring that the analysis tool runs efficiently while handling complex program structures.

## Solutions

1. **Def-Use Chain Tracking**: Implement recursive functions to trace the definition-use chain of variables affecting key points in the program.
2. **Loop and Conditional Analysis**: Analyze loops and conditional statements to determine which variables influence their behavior.
3. **I/O and API Semantics Integration**: Hardcode the semantics of relevant I/O APIs inside the compiler-based analysis for more accurate results in complex cases.
4. **JSON Output for Clarity**: Use JSON format to clearly present the seminal input features, specifying the variable name and its location in the code.

## Implementation Highlights

- **VariableInfo Structure**: Captures essential details of variables (name and line number).
- **JSON Output**: Presents results in a clear, structured format for easy analysis.
- **LLVM Pass Structure**: Utilizes LLVM’s infrastructure to analyze C programs effectively.
- **Def-Use Chain Analysis**: Identifies influential variables by tracing their usage and definitions.
- **I/O Function Identification**: Detects and handles input-related functions like `scanf`, `fopen`, and `getc`.

### Test Files
1. **Example1:**
```
#include <stdio.h>
int main(){
   int id;
   int n;
   scanf("%d",  &n);
   int s = 0;
   id = n;
   for (int i=0;i<id;i++){
      s += 1;
   }
   printf("id=%d; sum=%d\n", id, n); 
}
```
1. **Example2:**
```
#include <stdio.h>

int main() {
    char str1[1000]; 
    FILE *fp = fopen("file.txt", "r"); 
    char c;
    int len = 0;
    if (fp == NULL) {
        printf("Failed to open file\n");
        return 1;
    }
    while (1) {
        c = getc(fp);
        if (c == EOF || len >= 999) break;
        str1[len++] = c;
    }
    str1[len] = '\0'; 
    printf("%s\n", str1);
    fclose(fp); 
    return 0;
}

```
## Test output: 
1. **Example1 output:**
```
Seminal Input Feature: Key variable: n, Line: 4
```
1. **Example2 output:**
```
Seminal Input Feature: Key variable: fp, Line: 5
```
## Conclusion

This LLVM-based tool significantly automates the process of identifying seminal input features in C programs. By focusing on key points such as branching decisions and function pointers, it provides valuable insights into how inputs influence program behavior. This tool not only aids in optimizing and debugging applications but also contributes to better understanding complex program structures, thereby enhancing the overall software development process.