#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <network.h>
#include <debug.h>
#include <errno.h>
#include <wiiuse/wpad.h>
#include <unistd.h>
#include <fat.h>
#include <fcntl.h>


static void *xfb = 0;
static GXRModeObj *rmode = 0;

char *replaceWord(const char *s, const char *oldW,
                                 const char *newW){
    char *result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);

    // Counting the number of times old word
    // occur in the string
    for (i = 0; s[i] != '\0'; i++)
    {
        if (strstr(&s[i], oldW) == &s[i])
        {
            cnt++;

            // Jumping to index after the old word.
            i += oldWlen - 1;
        }
    }

    // Making new string of enough length
    result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1);

    i = 0;
    while (*s)
    {
        // compare the substring with the result
        if (strstr(s, oldW) == s)
        {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }

    result[i] = '\0';
    return result;
}

char* php_strstr(const char* haystack, const char* needle, char** before, char** after) {
    size_t len = strlen(needle);
    if (len == 0) {
        *before = 0;
        *after = 0;
        return (char*)haystack;
    }
    const char* start = strstr(haystack, needle);
    if (start == 0) {
        *before = 0;
        *after = 0;
        return 0;
    }
    size_t before_len = start - haystack;
    size_t after_len = strlen(start + len);
    *before = malloc(before_len + 1);
    strncpy(*before, haystack, before_len);
    (*before)[before_len] = '\0';
    *after = malloc(after_len + 1);
    strncpy(*after, start + len, after_len);
    (*after)[after_len] = '\0';
    return (char*)start;
}


void *initialise();
void *httpd (int port);

static lwp_t thread = LWP_THREAD_NULL;

void split(char *str, char *delim) {
    char *token = strtok(str, delim);
    while (token != 0) {
        printf("%s\n", token);
        token = strtok(0, delim);
    }
}



//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	s32 ret2;
	char* result;
	char localip[16] = {0};
	char gateway[16] = {0};
	char netmask[16] = {0};

	xfb = initialise();
    if (!fatInitDefault()) {
		printf("Error al iniciar fat.\n");
		return 1;
	}
    else{
        printf("Fat iniciado correctamente.");
    }

	printf("Configurando red ...\n");

	// Configure the network interface
	ret2 = if_config ( localip, netmask, gateway, TRUE, 20);
	printf ("\n\n\n\n\n\n\n\n");
	if (ret2>=0) {
		printf ("network configured, ip: %s, gw: %s, mask %s\n", localip, gateway, netmask);
	} else {
		printf ("network configuration failed!\n");
	}

	int sock;
    struct sockaddr_in server;
    char server_reply[2000];

    // Crea el socket
    sock = net_socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Error al crear el socket");
    }

    // Especifica la dirección del servidor
    server.sin_addr.s_addr = inet_addr("192.168.1.184"); // Dirección IP del servidor
    server.sin_family = AF_INET;
    server.sin_port = htons(21); // Puerto del servidor

	//net_send(csock, http_200, strlen(http_200), 0);

    // Conecta al servidor
    if (net_connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Error al conectar");
        return 0;
    }


    if (net_recv(sock, server_reply, 2000, 0) < 0) {
        printf("Error al recibir la respuesta");
        return 0;
    }

    printf("Respuesta: %s\n", replaceWord(server_reply,"\r",""));
    //USER
	if (net_send(sock, "USER usu\r\n", strlen("USER usu\r\n"), 0) < 0){
		printf("Error USER command.");
		return 0;
	}
	
    if (net_recv(sock, server_reply, 2000, 0) < 0) {
        printf("Error al recibir la respuesta");
        return 0;
    }
	printf("Respuesta: %s\n", replaceWord(server_reply,"\r",""));

	//PASSWORD
	if (net_send(sock, "PASS contra\r\n", strlen("PASS contra\r\n"), 0) < 0){
		printf("Error PASS command.");
		return 0;
	}

    if (net_recv(sock, server_reply, 2000, 0) < 0) {
        printf("Error al recibir la respuesta");
        return 0;
    }
	printf("Respuesta: %s\n", replaceWord(server_reply,"\r",""));

	//TYPE I
	if (net_send(sock, "TYPE I\r\n", strlen("TYPE I\r\n"), 0) < 0){
		printf("Error TYPE command.");
		return 0;
	}

    if (net_recv(sock, server_reply, 2000, 0) < 0) {
        printf("Error al recibir la respuesta");
        return 0;
    }
	printf("Respuesta: %s\n", replaceWord(server_reply,"\r",""));

	//PASV
	if (net_send(sock, "PASV\r\n", strlen("PASV\r\n"), 0) < 0){
		printf("Error PASV command.");
		return 0;
	}

    if (net_recv(sock, server_reply, 2000, 0) < 0) {
        printf("Error al recibir la respuesta");
        return 0;
    }
	result = replaceWord(server_reply,"227 Entering passive mode (192,168,1,184,","");
	result = replaceWord(result, ".", "");
	result = replaceWord(result, ")", "");
	result = replaceWord(result,","," ");

    printf(result);

    char needle[] = " ";
    char *before, *after;
    php_strstr(result, needle, &before, &after);

	int port = atoi(before) * 256 + atoi(after);
	printf("Port: %d\n",port);
	LWP_CreateThread(&thread,httpd,port,0, 100*1024,50);

                        
	printf("Respuesta: %s\n", replaceWord(server_reply,"\r",""));
	//CWD
	if (net_send(sock, "CWD Users/Edgar/Documents\r\n", strlen("CWD Users/Edgar/Documents\r\n"), 0) < 0){
		printf("Error CWD command.");
		return 0;
	}

    if (net_recv(sock, server_reply, 2000, 0) < 0) {
        printf("Error al recibir la respuesta");
        return 0;
    }
	printf("Respuesta: %s\n", replaceWord(server_reply,"\r",""));

    //LIST
	if (net_send(sock, "LIST\r\n", strlen("LIST\r\n"), 0) < 0){
		printf("Error LIST command.");
		return 0;
	}

    if (net_recv(sock, server_reply, 2000, 0) < 0) {
        printf("Error al recibir la respuesta");
        return 0;
    }
	printf("Respuesta: %s\n", replaceWord(server_reply,"\r",""));

    //RETR 
	if (net_send(sock, "RETR test_ftp.txt\r\n", strlen("RETR test_ftp.txt\r\n"), 0) < 0){
		printf("Error RETR command.");
		return 0;
	}

    if (net_recv(sock, server_reply, 2000, 0) < 0) {
        printf("Error al recibir la respuesta");
        return 0;
    }
	printf("Respuesta: %s\n", replaceWord(server_reply,"\r",""));

    //NOOP 
	if (net_send(sock, "NOOP\r\n", strlen("NOOP\r\n"), 0) < 0){
		printf("Error NOOP command.");
		return 0;
	}

    if (net_recv(sock, server_reply, 2000, 0) < 0) {
        printf("Error al recibir la respuesta");
        return 0;
    }
    printf("Respuesta: %s\n", replaceWord(server_reply,"\r",""));

    //QUIT
	if (net_send(sock, "QUIT\r\n", strlen("QUIT\r\n"), 0) < 0){
		printf("Error QUIT command.");
		return 0;
	}

    if (net_recv(sock, server_reply, 2000, 0) < 0) {
        printf("Error al recibir la respuesta");
        return 0;
    }
	printf("Respuesta: %s\n", replaceWord(server_reply,"\r",""));
    net_close(sock);



	while(1) {

		VIDEO_WaitVSync();
		WPAD_ScanPads();

		int buttonsDown = WPAD_ButtonsDown(0);

		if (buttonsDown & WPAD_BUTTON_HOME) {
			exit(0);
		}
	}

	return 0;
}


void *httpd (int port) {

    FILE *f;
    int sock;
    struct sockaddr_in server;
    char server_reply[20000];
    printf("Puerto: %d",port);
    // Crea el socket
    sock = net_socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Error al crear el socket");
    }

    // Especifica la dirección del servidor
    server.sin_addr.s_addr = inet_addr("192.168.1.184"); // Dirección IP del servidor
    server.sin_family = AF_INET;
    server.sin_port = htons(port); // Puerto del servidor


    // Conecta al servidor
    
    int ret = net_connect(sock, (struct sockaddr *)&server, sizeof(server));
    printf("Ret: %d",ret);
    if (ret < 0) {
        printf("Error al conectar");
        return NULL;
    }

    
       if (net_recv(sock, server_reply, 20000, 0) < 0) {
           printf("Error al recibir la respuesta");
           return NULL;
       }
       printf("Datos: %s",server_reply);
       int compare = strcmp(server_reply,"\0");
       printf("Compare: %d",compare);
       if (strcmp(server_reply,"\0") != 0) {
        
           f = fopen("sd:/test_ftp.txt","w");
           if (f==NULL){
               printf("No se ha podido acceder");
               return NULL;
           }
           fputs(server_reply,f);
           fclose(f);


           printf("Conectado servidor datos: %s\n", server_reply);
       }
    
    
    return 0;
	}


void *initialise() {


	void *framebuffer;

	VIDEO_Init();
	WPAD_Init();

	rmode = VIDEO_GetPreferredMode(0);
	framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(framebuffer,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(framebuffer);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	return framebuffer;

}
