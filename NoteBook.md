# Notebook

Here I am going to log what I learned and the blocking points of this development.

**Table of content**
1. [Disassembling](#Disassembling)
2. [Compilation](#Compilation)
3. [Design] (##Design)


## Disassembling

I start with a simple program converting a binary game boy rom file into assembly code.

### Rom content

First it is funny to note that printing the rom as ascii makes the game dialogs visible:

```Tu as le Violon des Vagues!�Tu as la Conque de l^Ecume!�Tu as la Cloche des Algues!�Tu as la Harpe  du Reflux!�Tu as le        Xylophone Marin!�Tu as le ��anglede Corail!�Tu as l^Orgue   de l^Embellie!�Tu as le Tambourdes Mar*es!�Si tu vois      les pointes,    pense % utiliserton Bouclier.�D^abord un lapinet en dernier,  un spectre...�Si loin...      Ne crains rien. Fonce et vole!�La lueur des    tuiles sera     ton guide...�Plonge l% o=    se croisent     les lumi+res    des flambeaux...�Fracasse        le mur          des Yeux        du Masque!�Le r*bus est    r*solu si les 4 piliers tombent.�Comble tous     les trous avec  le roc qui rouleet cette � est  la solution!�Il y a une      inscription sur le Marbre. Tu nepeux pas la lirecar un Fragment est manquant.```

The rom can also be displayed in hexadecimal with the `hexdump filename` command. **Each assembly instruction is encoded in one to three bytes**. The first byte is an opcode identifying the instruction and there can by bytes following for the instruction arguments. Thus, hte first byte make us able to know how many arguments we should expect and which byte is the following opcode. For example, the following bytes `c3 72 28 7e 1e 02` will be desassembled as:

    Bynary code 	Corresponding instruction
    c3 72 28      JP 2872
    7e            LD A (HL)
    1e 02         LD E 02

### Binary files

I learned that a binary file can't be edited in any text editor because it will probably store '1001 1110' as a string... GEdit seems to store the encoding of files in its own cache since the encoding is not stored in the file itself. I do not know how sublimetext decides to decode a file. I suspect it uses the frequency of the bytes to guess what encoding is used. Indeed, some bytes are recurent in a text file while other never appear.

In C++, reading in a binary file can be done as follow:
    
	std::ifstream myfilestream(argv[1], std::ios::binary | std::ios::in);
	std::istreambuf_iterator<char> file_it_start(myfilestream), file_it_end;
	std::vector<char> char_vect(file_it_start, file_it_end);

Thus, the binary file can be stored in a vector of bytes.

### Cartridge
I started with a Zelda cartridge since I was desassembled on http://computerarcheology.com/ and I notices the bytes where 7 times more numerous than the expected 32kB size. The cartridge actually contains several *banks* that can be plugged in the CPU memory between 4000 and 8000.

### Iterating through the bytes
I used an iterator at first but this cause me some troubles. Indeed, when I jump several opbytes, I can get out the vector of byte without the system noticing it. So I have to check on every iteration that I am still it the vector (see below) and I find it ugly... I plan to use the `[]` operator for vectors with an index when I will emulate the CPU. Moreover, the CPU will have a program counter `PC` register that will serve as an array index. 

	std::vector<char>::iterator it = char_vect.begin();

	while (it!=char_vect.end()){
		try{
			std::cout<<"0x"<<std::hex<<std::setfill('0') << std::setw(2)<<(int)*it<<" ";
			it+=disassemble(it);
			//check that it do not overpass end
			if (size_t(it - char_vect.begin())>char_vect.size()){
				throw "Try to disassemble instruction outside of memory bounds";
			}
		}
		catch(const char * c){
			std::cerr << "Fatal error: " << c << std::endl;
			return 1;
		}
	}

## Compilation

First, I compiled my code using:

    g++ -Wall -funsigned-char src/disassembler.cpp -o build/disassembler`

With more dependancies, you need then:

    g++ -Wall -funsigned-char src/Cpu.cpp src/Cpu.hpp -c -o build/Cpu.o 
    g++ -Wall -funsigned-char src/emulator.cpp build/Cpu.o -o build/emulator 

Which becomes complicated with a lot of targets (emulator, disasembler, test, etc...) hence the need for a makefile.

### Compilation options

 * `-Wall` : Makes g++ really picky on unclear code, wrong types or mistakes. This should be used when developping to prevent mistakes and invisible bugs.

 * `-c` : Just create an object file, not an executable
 * `-o` : Specify the output file name
 * `-funsigned-char` : On my system, `char` is interpreted as `signed char` so I need to force the interpretation es `unsigned char` since I wand to work with bytes.

### Debuging

**gdb**

**profiler**

### Makefile

I wrote a Makefile which is quite self-explaining. 
The main link I used was [this one](https://stackoverflow.com/questions/2481269/how-to-make-a-simple-c-makefile), which goes through different makefile levels of complexity.

The key is to use variables, here are some internal variables to makefile: 

|$@ 	|Target name								|
|$< 	|First dependancy name				|
|$^ 	|Dependancies list						|
|$? 	|List of the dependancies more recent than the target|
|$* 	|Name of the file without surfix			|


## Design

### General design

The memory will be represented by a vector to provide easy access.

We have two main classes, one handles the dissassembling and the other the CPU emulation.

For the Disassembling class, I decided to use iterators to go through the rom, because it enables passing only one argument instead of the vector of bytes and an index. This brings the issue of going out of bounds if we do not check that `it<myvector.end()`.

In the Cpu class, it was more natural to use an index since the program counter plays that roll and it can be accessible at any moment like the memory vector since they are class member. The rom could also have been made part of the Disassemble class, but I wanted this class to be independant from the data such that in one main, it can disassemble anything.

**How to know if after executing an instruction we should increment the program counter with the number of opbytes or not?**

Indeed, with jump instruction, we do not want to increment the pc with the opbytes after jumping.

We could increment the pc when calling nn() op1() etc... but then we would risk to mess up the program when calling nn() in an other case. Moreover, what if op2 is called before op1.

Instead we set an opbytes number for each opcode and increment the pc only if pc has not been changed by the instruction executed.

### Constants

I finally got the need for constants, especialy for class methods. For example, in the following function:`inline uint16_t word(const uint8_t lb, const uint8_t hb){ return lb + (hb<<8);}`

I just want to create a word from two bytes and don't want to risk that any of those bytes could be modified in the function. Here there is no risk since they are passed by value but it would be an issue if they were passed by reference, you get the idea.

Another example: `void Cpu::print_mem(int start_index, int byte_nb) const;` 
In the following fonction, I don't want the function to print the memory to be able to modify my cpu state. The const at the end ensures me of that which make a protection against mistakes.

I think this project is a good way to understand how this is important because it is easy to understand that if a function modifies a tiny part of the Cpu at some time, while it is not supposed to, it will mess up the entire emulation and be a nightmare to debug.

## Assembly code

### Difference between (HL) and HL

According to [this link](http://furrtek.free.fr/?a=gbasm), (HL) is the value stored at adress contained in HL. This is the **indirect adress mode**.

### Interruption

Some instruction enable "interruptions", I need to investigate this point....

## Documentation

I use Doxygen which I installed with `apt-get install doxygen` . Then I needed to create a configuration file which works as a Makefile with the command `doxygen -g Doxyfile` where Doxyfile is optional and is the name of the config file. The documentation can be generated with the command `doxygen Doxyfile`. The markdown files of the project are even included in the documentation!