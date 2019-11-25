
## **Checkers game**

Basic checkers game write in C.

- 23/11/2019
  * Separete server base into another repository.

- 04/11/2019 
  * Update base of server and client.

- 01/11/2019: Estimated time spend: 2 hours. (not continuos)
  * Develop SD project integration and and new view board.

- 28/10/2019: Estimated time spend: ~6 hours. (continuos)
  * Generalize input logic, add menu and add commands.

- 24/10/2019: Estimated time spend: ~1 hour.
  * Simple updates and refactor.

- 23/10/2019: Estimated time spend: ~5 hours.
  * Fix bugs and stylized game.

- 22/10/2019: Estimated time spend: 3 hours (not continuos).
  * Fix some bugs.

- 21/10/2019: Estimated time spend: 8 hours (not continuos).
  * Create game.


### **Compile and run**
> gcc game.c -o game -lpthread && ./game

### **Compile and run [DEBUG mode]**
> gcc game.c -o game -D DEBUG -lpthread && ./game

### **Observations***
> **!Warning!**: Files "game.v[*].c" can has errors.  

## Archictecture

![alt text](https://github.com/danielbom/Checkers/blob/master/checkers_arch2.png "Checkers")   

For more details about the SD project:  
[Documentation](https://github.com/danielbom/Checkers/blob/master/server/README.md)  
