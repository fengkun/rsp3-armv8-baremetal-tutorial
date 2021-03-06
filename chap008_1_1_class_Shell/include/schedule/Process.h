/*
 * Progress.h
 *
 *  Created on: Mar 9, 2018
 *      Author: 13774
 */

#ifndef INCLUDE_SCHEDULE_PROGRESS_H_
#define INCLUDE_SCHEDULE_PROGRESS_H_

#include <memory/VirtualMap.h>
#include <memory/ProcessVirtualMap.h>
#include <schedule/forward.h>
#include <io/Output.h>
#include <filesystem/VirtualFile.h>
#include <memory/MemoryManager.h>

// 提供一个统一的view
// 系统层面的Process，不是用户层面的Process
class Process{
public:
	using _D = VirtualMemoryDefinition;
	static constexpr size_t REGISTER_NUM=31;
	enum Status{
		CREATED, // 创建成功
		READY,  // 进程可以运行
		RUNNING,    // 进程正在进行
		BLOCKED,    // 进程阻塞，正在等待某个事件
		STOPPED,    // 进程已经执行完毕，但是使用的资源还没有回收
		DESTROYED, //已经将进程回收了

		STATUS_NUM
	};
	Process(ProcessLink *     parent ,size_t codePages,size_t kernelStackPages,
			size_t userStackPages, size_t freeRamPages);
	~Process();

	/**
	 * fork的实现
	 * fork进程之间默认是 父子关系，而不是兄弟关系
	 * 前置条件：必须保证Process不为：CREATED_INCOMPLETE, DESTROYED
	 * @param rhs
	 * 返回  CREATED_INCOMPLETE,或者CREATED的进程
	 */
	Process(const Process & rhs);
	Process& operator=(const Process &rhs)=delete;
	Process(Process &&rhs)=delete;
	Process& operator=(Process &&rhs)=delete;

	// 必须保证 *this!=proc，即必须是两个不同的进程，否则出现错误。
	void saveContextAndChange(Process &proc);// 系统中存在运行进程时使用


	// 注意：该过程保证不会发生同步异常
	void restoreContextAndExecute(); // 在系统中没有运行进程时使用(但是系统可以有阻塞进程)
	AS_MACRO Pid pid() const { return _pid;}
	uint64_t *      registers() { return _registers;}
	const uint64_t* registers() const { return _registers;}
	AS_MACRO const RegSP_EL0 &SPEL0() const { return _spel0;}
	AS_MACRO Status status() const { return _status;}
	AS_MACRO void   status(Status status) { _status=status;}
	AS_MACRO const RegELR_EL1& ELR() const { return _elr;}
	AS_MACRO RegELR_EL1& ELR() { return _elr;}
	AS_MACRO RegSPSR_EL1& SPSR() { return _spsr;}
	AS_MACRO const RegSPSR_EL1& SPSR() const { return _spsr;}
	AS_MACRO ProcessLink * parent() { return _parent;}
	AS_MACRO const ProcessLink * parent()const { return _parent;}
	AS_MACRO const ProcessVirtualMap& virtualMap()const { return _vmap;}
	AS_MACRO ProcessVirtualMap& virtualMap() { return _vmap;}
	AS_MACRO const RegTTBR0_EL1& TTBR0() const { return _ttbr0;}
	AS_MACRO const Output & out() const {return _stdout;}
	AS_MACRO Output & out()  {return _stdout;}
	AS_MACRO const VirtualFile* workingDir() const { return _workingDir;}
	AS_MACRO VirtualFile* workingDir() { return _workingDir;}
	// 必须保证dir不为空，且有效
	AS_MACRO void workingDir(VirtualFile* dir) { _workingDir=dir;}

	// FIXME 一般而言，这里的内存管理器与进程使用的内存管理器是不同的
	//        但是它们却共同管理同一块内存区域，因此必须保证互斥
	//        但是，在异步中断中，一般不会调用内存管理；在同步中断中，内核
	//        调用内存管理不会发生在内存分配中，只会发生在前后，因此可以保证
	//        串行。如果将来这样的条件不满足，则需要使用同步。
	//        因为内存只在TTBR0启用的情况下有效，所以mman不应当在其他进程中
	//        使用，只在当前运行的进程中使用。
	//     使用与用户态共享的内存是安全的，因为其影响仅仅局限于用户态内存。
	AS_MACRO const MemoryManager & mman() const { return _mman;}
	AS_MACRO MemoryManager & mman() { return _mman;}
	AS_MACRO int   exitCode()const{ return _exitCode;}
	AS_MACRO void  exitCode(int exitCode) { _exitCode=exitCode;}

	static   const char * statusToString(Status s);


private:
	// ARMv8 特有的结构
	uint64_t       _registers[REGISTER_NUM];// 注意, _registers[0] 通常作为返回值
	RegTTBR0_EL1    _ttbr0 ;
	RegSP_EL0       _spel0;
	size_t          _spel1;
	RegELR_EL1      _elr ;
	RegSPSR_EL1     _spsr ;


	// 其他部分
	Pid               _pid;
	Status            _status ;
	ProcessLink *     _parent ;
	ProcessVirtualMap _vmap ;
	Output            _stdout;
	VirtualFile    *  _workingDir;
	MemoryManager     _mman;
	int               _exitCode ;// 退出状态

};


#endif /* INCLUDE_SCHEDULE_PROCESS_H_ */
