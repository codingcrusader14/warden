.section ".vector_table"
.global _vector_table_entry
.align 7

// each vector table entry is 128B
// there are a total of 16 entries totaling 2KB
// offsets entries begin at 0x000 and end at 0x780

_vector_table_entry: // start of vector table

.align 7
sync_current_el_sp0:
        b .
        
.align 7
irq_current_el_sp0: 
        b .
        
.align 7
fiq_current_el_sp0:
        b .
        
.align 7
serror_current_el_sp0:
        b .
        
.align 7
sync_current_el_spx:
        b .
        
.align 7
irq_current_el_spx:
        b .
        
.align 7
fiq_current_el_spx:
        b .
        
.align 7
serror_current_el_spx:
        b .
        
.align 7
sync_lower_el_aarch64:
        b .
        
.align 7
irq_lower_el_aarch64:
        b .
        
.align 7
fiq_lower_el_aarch64:
        b .
        
.align 7
serror_lower_el_aarch64:
        b .
        
.align 7
sync_lower_el_aarch32:
        b .
        
.align 7
irq_lower_el_aarch32:
        b .
        
.align 7
fiq_lower_el_aarch32:
        b .
        
.align 7
serror_current_el_aarch32:
        b .

             
 
        
