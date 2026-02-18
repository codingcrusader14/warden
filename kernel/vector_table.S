.section ".vector_table"
.global _vector_table_entry

// each vector table entry is 128B
// there are a total of 16 entries totaling 2KB
// offsets entries begin at 0x000 and end at 0x780

.balign 0x800
_vector_table_entry: // start of vector table

.balign 0x80
sync_current_el_sp0:
        b kernelvec
        
.balign 0x80
irq_current_el_sp0: 
        b .
        
.balign 0x80
fiq_current_el_sp0:
        b .
        
.balign 0x80
serror_current_el_sp0:
        b .
        
.balign 0x80
sync_current_el_spx:
        b kernelvec
        
.balign 0x80
irq_current_el_spx:
        b .
        
.balign 0x80
fiq_current_el_spx:
        b .
        
.balign 0x80
serror_current_el_spx:
        b .
        
.balign 0x80
sync_lower_el_aarch64:
        b .
        
.balign 0x80
irq_lower_el_aarch64:
        b .
        
.balign 0x80
fiq_lower_el_aarch64:
        b .
        
.balign 0x80
serror_lower_el_aarch64:
        b .
        
.balign 0x80
sync_lower_el_aarch32:
        b .
        
.balign 0x80
irq_lower_el_aarch32:
        b .
        
.balign 0x80
fiq_lower_el_aarch32:
        b .
        
.balign 0x80
serror_current_el_aarch32:
        b .

             
 
        
