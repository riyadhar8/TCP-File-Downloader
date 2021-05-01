#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

int ReadHttpStatus(int sock){
    char c;
    char buff[1024]="",*ptr=buff+1;
    int bytes_received, status;
    while(bytes_received = recv(sock, ptr, 1, 0)){
        if(bytes_received==-1){
            printf("Error in reading HTTP status.\n");
            exit(1);
        }

        if((ptr[-1]=='\r')  && (*ptr=='\n' )) break;
        ptr++;
    }
    *ptr=0;
    ptr=buff+1;

    sscanf(ptr,"%*s %d ", &status);

    printf("%s\n",ptr);
    printf("status=%d\n",status);
    return (bytes_received>0)?status:0;

}

int ParseHeader(int sock){
    char c;
    char buff[1024]="",*ptr=buff+4;
    int bytes_received, status;
    while(bytes_received = recv(sock, ptr, 1, 0)){
        if(bytes_received==-1){
            printf("Error while Parsing Header");
            exit(1);
        }

        if(
            (ptr[-3]=='\r')  && (ptr[-2]=='\n' ) &&
            (ptr[-1]=='\r')  && (*ptr=='\n' )
        ) break;
        ptr++;
    }

    *ptr=0;
    ptr=buff+4;
    

    if(bytes_received){
        ptr=strstr(ptr,"Content-Length:");
        if(ptr){
            sscanf(ptr,"%*s %d",&bytes_received);

        }else
            bytes_received=-1;

    }
    return  bytes_received ;

}

int ReadHttpStatus2(SSL* sock){
    char c;
    char buff[3000]="",*ptr=buff+1;
    int bytes_received, status;
    while(bytes_received = SSL_read(sock, ptr, 1)){
        if(bytes_received==-1){
            printf("Error in reading HTTP status.\n");
            exit(1);
        }

        if((ptr[-1]=='\r')  && (*ptr=='\n' )) break;
        ptr++;
    }
    *ptr=0;
    ptr=buff+1;

    sscanf(ptr,"%*s %d ", &status);

    printf("%s\n",ptr);
    printf("status=%d\n",status);
    return (bytes_received>0)?status:0;

}


int ParseHeader2(SSL *sock){
    char c;
    char buff[3000]="",*ptr=buff+4;
    int bytes_received, status;
    while(bytes_received = SSL_read(sock, ptr, 1)){
        if(bytes_received==-1){
            printf("Error while Parsing Header");
            exit(1);
        }

        if(
            (ptr[-3]=='\r')  && (ptr[-2]=='\n' ) &&
            (ptr[-1]=='\r')  && (*ptr=='\n' )
        ) break;
        ptr++;
    }

    *ptr=0;
    ptr=buff+4;

    if(bytes_received){
        ptr=strstr(ptr,"Content-Length:");
        if(ptr){
            sscanf(ptr,"%*s %d",&bytes_received);

        }else
            bytes_received=-1; 

    }
    return  bytes_received ;

}

void file_name(char *string1) {
    
    int len = strlen(string1);

    int left = 0;
    int right = len - 1;
    
    for(int i = left; i < right; i++) {
        char tmp = string1[i];
        string1[i] = string1[right];
        string1[right] = tmp;
        right--;
    }

    sscanf(string1, "%[^/]", string1);

    len = strlen(string1);

    left = 0;
    right = len - 1;
    
    for(int i = left; i < right; i++) {
        char tmp = string1[i];
        string1[i] = string1[right];
        string1[right] = tmp;
        right--;
    }
}

//http file
void http(char *string1) {
    char domain[1024];
    char path[1024];

    sscanf(string1, "http://%[^/]/%[^\n]", domain, path);

    char fileName[1024];

    for (int i=0; i<strlen(path); i++) {
        fileName[i] = path[i];
    }

    //printf("Your domain is: %s\n", domain);

    file_name(fileName);

    //printf("File will be saved under: %s\n", fileName);

    int sock;
    int bytes_received;  
    char sendb[3000];
    char recvb[1024];
    char *p;
    struct sockaddr_in server_addr;
    struct hostent *he;

    he = gethostbyname(domain);
    if (he == NULL){
       printf("Unreachable or wrong URL.\n");
       exit(1);
    } else {
        printf("HTTP URL.\n");
    }

    printf("Your domain is: %s\n", domain);
    printf("File will be saved under: %s\n", fileName);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0))== -1){
       perror("Socket creation failed.\n");
       exit(1);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;     
    server_addr.sin_port = htons(80);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);

    printf("----- Connecting.\n");
    if (connect(sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1){
       perror("Failed to Connect.\n");
       exit(1); 
    }

    printf("----- Sending data.\n");

    snprintf(sendb, sizeof(sendb), "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", path, domain);

    if(send(sock, sendb, strlen(sendb), 0)==-1){
        printf("Failed to send.\n");
        exit(2); 
    }
    printf("----- Data sent.\n");  

    printf("----- Receiving data.\n");

    int contentlengh;

    if(ReadHttpStatus(sock) && (contentlengh=ParseHeader(sock))){

        int bytes=0;
        FILE* fp=fopen(fileName,"wb");

        while(bytes_received = recv(sock, recvb, 1024, 0)){
            if(bytes_received==-1){
                printf("Error in receiving.\n");
                exit(3);
            }


            fwrite(recvb,1,bytes_received,fp);
            bytes+=bytes_received;
            if(bytes==contentlengh)
                break;
        }
        printf("----- Saving data.\n");
        fclose(fp);
    }

    close(sock);
    printf("Done.\n\n");

}

//https files
void https(char *string1) {

    char domain[1024];
    char path[1024];

    //separating the domain and the path
    sscanf(string1, "https://%[^/]/%[^\n]", domain, path);

    char fileName[1024];

    //the name of the file that is saved would be the same as the name of the path
    for (int i=0; i<strlen(path); i++) {
        fileName[i] = path[i];
    }

    //to get the string after the last '/' in the URL
    file_name(fileName);

    int sock;
    int bytes_received;  
    char sendb[3000];
    char recvb[3000];
    char *p;
    struct sockaddr_in server_addr;
    struct hostent *he;

    //connecting to the URL
    he = gethostbyname(domain);
    if (he == NULL){
       printf("Unreachable or wrong URL.\n");
       exit(1);
    } else {
        printf("HTTPS URL.\n");
    }

    printf("Your domain is: %s\n", domain);
    printf("File will be saved under: %s\n", fileName);

    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    SSL_library_init();

    //socket creation
    if ((sock = socket(AF_INET, SOCK_STREAM, 0))== -1){
       perror("Socket creation failed.\n");
       exit(1);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;     
    server_addr.sin_port = htons(443);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);

    //connecting to socket
    printf("----- Connecting.\n");
    if (connect(sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1){
       perror("Failed to Connect.\n");
       exit(1); 
    }

    SSL* ssl;
    SSL_CTX *ctx = SSL_CTX_new (SSLv23_client_method ());
    SSL* conn = SSL_new(ctx);
    SSL_set_fd(conn, sock);

    if (SSL_connect(conn) != 1) {
        int x = SSL_connect(conn);
        printf("%d\n", x);
        printf("%d\n", SSL_get_error(conn, x));
        printf("Could not connect.\n");
        exit(1);
    } else {
        printf("----- Connected.\n");
    }


    printf("----- Sending data.\n");

    snprintf(sendb, sizeof(sendb), "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, domain);

    //printf("%s\n", sendb);

    if(SSL_write(conn, sendb, strlen(sendb))==-1){
        printf("Failed to send.\n");
        exit(2); 
    }
    printf("----- Data sent.\n");  

    printf("----- Receiving data.\n");

    int contentlengh;

    if(ReadHttpStatus2(conn) && (contentlengh=ParseHeader2(conn))){

        int bytes=0;
        FILE* fp=fopen(fileName,"wb");

        while(bytes_received = SSL_read(conn, recvb, 3000)){
            if(bytes_received==-1){
                printf("Error in receiving.\n");
                exit(3);
            }


            fwrite(recvb,1,bytes_received,fp);
            bytes+=bytes_received;
            if(bytes==contentlengh)
                break;
        }
        printf("----- Saving data.\n");
        fclose(fp);
    }

    close(sock);
    SSL_free(conn);
    SSL_CTX_free(ctx);
    printf("Done.\n");

}

int main(int argc, char* argv[]) {
    if (argc<2) {
        printf("Please enter the URL from which the file is to be downloaded.\n");
        exit(1);
    }

    if (argv[1][4] == ':') {
        //printf("HTTP URL.\n");
        http(argv[1]);
    }

    else if (argv[1][4] == 's') {
        //printf("HTTPS URL.\n");
        https(argv[1]);
    }

    else {
        printf("Invalid URL.\n");
    }

    return 0;
}