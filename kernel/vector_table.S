.section ".vector_table"
.global _vector_table_entry

// each vector table entry is 128B
// there are a total of 16 entries totaling 2KB
// offsets entries begin at 0x000 and end at 0x780

.balign 0x800
_vector_table_entry: // start of vector table

.balign 0x80
sync_current_el_sp0:
        bl sync_exception
        
.balign 0x80
irq_current_el_sp0: 
        bl irq_exception
        
.balign 0x80
fiq_current_el_sp0:
        bl fiq_exception
        
.balign 0x80
serror_current_el_sp0:
        bl system_exception
        
.balign 0x80
sync_current_el_spx:
        bl sync_exception

.balign 0x80
irq_current_el_spx:
        bl irq_exception
        
.balign 0x80
fiq_current_el_spx:
        bl fiq_exception
        
.balign 0x80
serror_current_el_spx:
        bl system_exception
        
.balign 0x80
sync_lower_el_aarch64:
        bl sync_exception
        
.balign 0x80
irq_lower_el_aarch64:
        bl irq_exception
        
.balign 0x80
fiq_lower_el_aarch64:
        bl fiq_exception
        
.balign 0x80
serror_lower_el_aarch64:
        bl system_exception
        
.balign 0x80
sync_lower_el_aarch32:
        bl sync_exception
        
.balign 0x80
irq_lower_el_aarch32:
        bl irq_exception
        
.balign 0x80
fiq_lower_el_aarch32:
        bl fiq_exception
        
.balign 0x80
serror_current_el_aarch32:
        bl system_exception

             
 
        
