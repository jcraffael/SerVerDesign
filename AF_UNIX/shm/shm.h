#ifndef SHM_H
#define SHM_H

int
read_from_shared_memory (char *mmap_key,
        char *buffer,
        unsigned int buff_size,
        unsigned int bytes_to_read);


int create_and_write_shared_memory (char *mmap_key, 
                                    char *value, 
                                    unsigned int size);

                        
#endif 