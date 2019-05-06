#include "Test.hpp"
#include "Cpu.hpp"

#include "Disassembler.hpp"
#include "utils.hpp"
#include <vector>
#include <string>


void test_jump(){

	Test t("jump_function",__FILE__,__FUNCTION__);

	//Init
	Cpu cpu;
	cpu.print_pc();
	cpu.print_sp();
	cpu.print_stack(3);	
	cpu.load_debug_cartridge("5f 01 02 03 04 05 06 07 08 09 10");
	cpu.print_mem(0x00,10);
	t.test_assert(cpu.get_pc()==00 ,__LINE__);
	t.test_assert(cpu.get_sp()==SP0,__LINE__);
	t.test_assert(cpu.get_stack(1)==0x00,__LINE__);
	t.test_assert(cpu.get_stack(0)==0x00,__LINE__);

	//Jump
	cpu.load_debug_cartridge("c3 02 01");
	cpu.print_mem(0x00,5);
	cpu.emulate();
	t.test_assert(cpu.get_pc(), 0x102 ,__LINE__);

	cpu.load_debug_cartridge("18 0a");
	cpu.print_mem(0x00,5);
	cpu.emulate();
	t.test_assert(cpu.get_pc(), 0x0a ,__LINE__);


	//call
	cpu.load_debug_cartridge("cd 10");
	cpu.print_mem(0x00,5);
	cpu.emulate();
	cpu.print_stack(3);	
	t.test_assert(cpu.get_pc(), 0x10 ,__LINE__);
	t.test_assert(cpu.get_sp(), SP0-2 ,__LINE__);
	t.test_assert(cpu.get_stack(0), 0x0 ,__LINE__);
	t.test_assert(cpu.get_stack(1), 0x3 ,__LINE__);

	//restart
	cpu.load_debug_cartridge("cf");
	cpu.emulate();
	t.test_assert(cpu.get_pc(), 0x08 ,__LINE__);
	t.test_assert(cpu.get_stack(0), 0x0 ,__LINE__);
	t.test_assert(cpu.get_stack(1), 0x1 ,__LINE__);

	//return
	cpu.load_debug_cartridge("c3 02 01 cd 10 c9");
	cpu.emulate();
	cpu.print_stack(4);	
	t.test_assert(cpu.get_pc(), 0x0102 ,__LINE__);
	t.test_assert(cpu.get_stack(0), 0x0 ,__LINE__);
	t.test_assert(cpu.get_stack(1), 0x0 ,__LINE__);

	cpu.load_debug_cartridge("c3 02 01 cd 10 cd 10 c9");
	cpu.emulate();
	cpu.print_stack(4);	
	t.test_assert(cpu.get_pc(), 0x010c ,__LINE__);
	t.test_assert(cpu.get_stack(0), 0x2 ,__LINE__);
	t.test_assert(cpu.get_stack(1), 0x1 ,__LINE__);

	//TODO Conditional jump (good behavior when not jumping?)

	t.result();
}

void test_load(){

	Test t("load_function",__FILE__,__FUNCTION__);
	Cpu cpu;

	cpu.load_debug_cartridge("16 05");
	cpu.emulate();
	t.test_assert(cpu.get_d(), 5 ,__LINE__);

	cpu.load_debug_cartridge("3e 11 26 0d 2e 0d 36 11");
	cpu.emulate();
	t.test_assert(cpu.get_a(), 17 ,__LINE__);
	t.test_assert(cpu.get_h(), (int)0xD ,__LINE__);
	t.test_assert(cpu.get_l(), (uint8_t)0xD ,__LINE__);
	t.test_assert(cpu.get_mem(0xDD), (uint16_t)0x11 ,__LINE__);
}

int main(){
	//test_jump();
	test_load();
	return 0;
}