package gdsl.decoder;

import gdsl.Gdsl;

/**
 * This class represents a decoded instruction.
 * 
 * @author Julian Kranz
 */
public class Instruction {
  private long insnPtr = 0;
  private Gdsl gdsl;
  private long size;
  
  private long heapRevision;

  /**
   * Get the address of the associated native instruction object
   * 
   * @return the value of the pointer
   */
  public long getInsnPtr () {
    if (heapRevision != gdsl.getHeapRevision())
      throw new RuntimeException("Heap expired");
    if (insnPtr == 0)
      throw new NullPointerException();
    return insnPtr;
  }

  /**
   * Get the size of the instruction
   * 
   * @return the size of the instruction
   */
  public long getSize () {
    return size;
  }
  
  /**
   * Get the associated {@link Gdsl} object
   * 
   * @return the associated {@link Gdsl} object
   */
  public Gdsl getGdsl () {
    return gdsl;
  }

  /**
   * Construct an instruction object
   * 
   * @param gdsl the associated {@link Gdsl} object
   * @param insnPtr the address of the native instruction object
   * @param size the size of the instruction
   */
  public Instruction (Gdsl gdsl, long insnPtr, long size) {
    this.gdsl = gdsl;
    this.heapRevision = gdsl.getHeapRevision();
    gdsl.lockHeap();
    this.insnPtr = insnPtr;
    this.size = size;
  }

  @Override public String toString () {
    return pretty(gdsl.getFrontendPtr(), gdsl.getGdslStatePtr(), getInsnPtr());
  }

  private native String pretty (long frontendPtr, long gdslStatePtr, long insnPtr);

  /**
   * Get the number of operands
   * 
   * @return the number of operands
   */
  public int operands () {
    return operands(gdsl.getFrontendPtr(), gdsl.getGdslStatePtr(), getInsnPtr());
  }

  private native int operands (long frontendPtr, long gdslStatePtr, long insnPtr);

  /**
   * Print an operand
   * 
   * @param operand the operand to print
   * @return the string representation of the operand
   */
  public String operandToString (int operand) {
    return prettyOperand(gdsl.getFrontendPtr(), gdsl.getGdslStatePtr(), getInsnPtr(), operand);
  }

  private native String prettyOperand (long frontendPtr, long gdslStatePtr, long insnPtr, int operand);
  
  public OperandType operandType(int operand) {
    return OperandType.fromGdslId(operandType(gdsl.getFrontendPtr(), gdsl.getGdslStatePtr(), getInsnPtr(), operand));
  }
  
  private native int operandType(long frontendPtr, long gdslStatePtr, long insnPtr, int operand);

  /**
   * Get the mnemonic of the instruction
   * 
   * @return the mnemonic of the instruction
   */
  public String mnemonic () {
    return mnemonic(gdsl.getFrontendPtr(), gdsl.getGdslStatePtr(), getInsnPtr());
  }

  private native String mnemonic (long frontendPtr, long gdslStatePtr, long insnPtr);
  
  @Override protected void finalize () throws Throwable {
    gdsl.unlockHeap();
    super.finalize();
  }
}
