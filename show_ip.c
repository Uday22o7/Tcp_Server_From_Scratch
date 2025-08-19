// Include all this headers.
#include<stdio.h>
#include<string.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>

// argc is a variable that hold count of element present in the argv array. 
int main(int agrc,char *argv[]){
    struct addrinfo hints, *res,*p; // Creating a variable of "addinfo" type.
    int status;
    char ipstr[INET6_ADDRSTRLEN]; // ipstr to store string form of ip after using ntop i.e network to presentation.
    
    if(agrc != 2){
        printf("Error not passed 2 args");
        return 1;
    }

    // memset is a func that will fill a block of memory with a particular value.
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    // refer to struct addinfo in socket s section at beej.us guide.
    
    if((status = getaddrinfo(argv[1],NULL,&hints,&res)) !=0){
        fprintf(stderr,"getaddrinfo error: %s\n", gai_strerror(status)); // fprintf used when ever you want to write data in file instead of stdout console. 
        return 2;
    }

    printf("IP address for %s:\n\n",argv[1]);

    p=res;
    while(p!=NULL){
        void *addr;
        char *ipver; // Here we are creating a string literal stored in read-only memory.They are assigned using "char *", allows referencing them without extra memory allocation. 

        if(p->ai_family == AF_INET){
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4"; // it is not a single character rather a string of multiple char.
        } else{
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(p->ai_family,addr,ipstr,sizeof(ipstr));
        printf(" %s:%s\n",ipver,ipstr);

        p = p->ai_next;
    }
    freeaddrinfo(res); //free linked list.
    return 0;
}


/*
echo $? 
if "0" = success, if non-zero = failure 

any argv also takes file name as input therefore argc=2, argv[0] is file_name and argv[1] will be the input.
*/