# __Algorithms and Data Structure project__

This project has been developed as part of the BSc thesis and represents the final exam of the course "Algoritmi e principi dell'informatica" ("Algorithms and principles of Computer Science") attended during my Bachelor's Degree (A.Y. 2019/20) at the Polytechnic University of Milan. The highest possible final grade has been achieved: 30 cum Laude.

## __Description__
Aim of this project was to design, implement and optimize a text editor, similar to Ed (Unix) but with the added possibility of undoing changes. The whole algorithm had to be written in C, and undergo a series of tests to verify its effectiveness and efficiency, in terms of both memory and time consumption.

## __Commands__
The implemented editor supports the following operations (with the assumption that X, Y represents two address specifiers such that X is less than or equal to Y):

|command   |structure|description|
|----------|:-------:|-----------|
|__Change__|X,Yc     |Changes the text present to the lines between X and Y|
|__Read__  |X,Yc     |Prints the lines between X and Y, included|
|__Delete__|X,Yc     |Deletes the lines between X and Y|
|__Undo__  |Xu       |Cancels a number of commands (c or d) equal to that specified|
|__Redo__  |Xr       |Undoes the effect of undo for a number of commands equal to  X starting with the current version|
|__Quit__  |q        |Ends the execution of the editor|

A line of text input to the editor can contain a maximum of 1024 characters. It is assumed that only correct commands are given to the editor. Further details can be found in the [project specification](Final_Exam_Project_Specification_2020.pdf).

## __Tests__
Every test set is made by different tests, each focusing on different aspect of the editor. Test cases were unknown during the development of the project. 
They can be found in the [test cases](/test_cases) folder.