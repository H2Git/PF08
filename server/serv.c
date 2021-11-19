#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/select.h>
#include "/usr/include/mysql/mysql.h"

#include <termios.h>
#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyACM0"
#define _POSIX_SOURCE 1 // POSIX compliant source
#define FALSE 0
#define TRUE 1
#define BUF_MAX 255
void error_handling(char *message);

#define MAX_CHAT 1024 // Clients' Send Size
#define MAX_SOCK 1024

#define DB_SERV "localhost"
#define DB_USER "root"
#define DB_PSWD "1234"
#define DB_NAME "0627QTArduino"
#define DB_PORT 3306
#define DB_SOCK (const char *)NULL /* use default socket name */

char *exit_strings = "/EXIT /Exit /exit /Q /q"; // String for clnt quit, one of them
char *confirm_echo = "Welcome Mafia Chat :) [connect confirm msg from serv]";

int num_user = 0;			// 채팅 참가자 수
int num_chat = 0;			// 지금까지 오간 대화의 수
int clnt_socks[MAX_SOCK];	// 채팅에 참가자 소켓번호 목록
char ip_list[MAX_SOCK][20]; // 접속한 ip목록
char buf[MAX_CHAT + 1];		//클라이언트에서 받은 메시지

MYSQL *connection = NULL;
MYSQL conn;
MYSQL_RES *sql_result;
MYSQL_ROW sql_row;
int query_stat;
char query[255];
char checking_msg[255];
char *sArr[6] = {
	NULL,
};

int flag = 0;

time_t c_tm;
struct tm tm;
int fd;

void QuitOnError(char *msg);
int ListenClients(int host, int port, int backlog); // socket create & listen
void *ManageSystem();								// func for thread
void AddClient(int sock, struct sockaddr_in *new_sockaddr);
void RemoveClient(int sock_index);	// 채팅 탈퇴 처리 함수
int GetMaxSockNum(int listen_sock); // 최대 소켓 번호 찾기

void database();
void endDB();
void printError(int line_num, MYSQL conn);
void printSuccess(int line_num, MYSQL conn);
void updateMemberDB();
void loginCustomerDB();
void loginManagerDB();
void searchLogDB();
void sendDB();
void LedOn();
void SpeakerOn();
void LcdOn();

int main(int argc, char *argv[])
{
	struct sockaddr_in clntaddr;
	int i, j, nbyte, accp_sock;
	int select_fds; // max + 1
	int lstn_sock;

	socklen_t addr_sz = sizeof(struct sockaddr_in);
	fd_set read_fds; //읽기를 감지할 fd_set 구조체
	pthread_t a_thread;

	if (argc != 2)
	{
		printf("사용법 :%s port\n", argv[0]);
		exit(0);
	}

	// ListenClients(host, port, backlog) 함수 호출
	lstn_sock = ListenClients(INADDR_ANY, atoi(argv[1]), 5);
	//스레드 생성
	pthread_create(&a_thread, NULL, ManageSystem, (void *)NULL);
	//db 연결
	database();

	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY);
	if (fd < 0)
	{
		perror(MODEMDEVICE);
		exit(-1);
	}
	struct termios tio;
	memset(&tio, 0, sizeof(tio));

	tcgetattr(fd, &tio);

	tio.c_cflag |= BAUDRATE;
	tio.c_cflag |= CS8;
	tio.c_cflag |= CLOCAL;
	tio.c_cflag |= CREAD;
	tio.c_iflag = IGNPAR;

	tcsetattr(fd, TCSANOW, &tio);

	for (;;)
	{
		FD_ZERO(&read_fds);
		FD_SET(lstn_sock, &read_fds);

		for (i = 0; i < num_user; i++)
			FD_SET(clnt_socks[i], &read_fds);

		select_fds = GetMaxSockNum(lstn_sock) + 1; // select_fds 재 계산

		if (select(select_fds, &read_fds, NULL, NULL, NULL) < 0)
			QuitOnError("select fail");

		if (FD_ISSET(lstn_sock, &read_fds))
		{
			accp_sock = accept(lstn_sock, (struct sockaddr *)&clntaddr, &addr_sz);
			if (accp_sock == -1)
				QuitOnError("accept fail");

			AddClient(accp_sock, &clntaddr);
			// send(accp_sock, confirm_echo, strlen(confirm_echo), 0);

			c_tm = time(NULL); //현재 시간을 받아옴
			tm = *localtime(&c_tm);
			//write(1, "\033[0G", 4); //커서의 X좌표를 0으로 이동
			printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
			fprintf(stderr, "\033[33m"); //글자색을 노란색으로 변경
			printf("채팅 사용자 1명 추가. 현재 참가자 수 = %d\n", num_user);
			fprintf(stderr, "\033[32m"); //글자색을 녹색으로 변경
			fprintf(stderr, "server>");	 //커서 출력
		}

		// 클라이언트가 보낸 메시지를 모든 클라이언트에게 전송
		for (i = 0; i < num_user; i++)
		{
			if (FD_ISSET(clnt_socks[i], &read_fds))
			{
				num_chat++; //총 대화 수 증가
				nbyte = recv(clnt_socks[i], buf, MAX_CHAT, 0);

				char *temp = strtok(buf, " "); //공백을 기준으로 문자열 자르기

				int i = 0;
				while (temp != NULL)
				{ //널이 아닐때까지 반복
					sArr[i] = temp;
					printf("\n받은 문자열 [%i]: %s\n", i, sArr[i]); // 출력
					i++;
					temp = strtok(NULL, " "); // 널문자를 기준으로 다시 자르기
				}

				if (*sArr[0] == '1')
				{
					updateMemberDB();
				}
				else if (*sArr[0] == '2')
				{
					loginCustomerDB();
				}
				else if (*sArr[0] == '3')
				{
					loginManagerDB();
				}
				else if (*sArr[0] == '4')
				{
					searchLogDB();
					sendDB();
				}
				else if (*sArr[0] == '5')
				{
					LedOn();
				}
				else if (*sArr[0] == '6')
				{
					SpeakerOn();
				}
				else if (*sArr[0] == '7')
				{
					LcdOn();
				}
				else
					printf("에러!!\n");

				if (nbyte <= 0) // 강제, 오류 연결 종료시 클라이언트 해제
				{
					RemoveClient(i);
					continue;
				}
				buf[nbyte] = 0;

				if (strstr(buf, exit_strings) != NULL) // 종료 문자를 클라이언트 종료
				{
					RemoveClient(i);
					continue;
				}

				// printf("\033[0G");			 //커서의 X좌표를 0으로 이동
				// fprintf(stderr, "\033[97m"); //글자색을 흰색으로 변경
				// printf("%s\n", buf);		 //메시지 출력
				// fprintf(stderr, "\033[32m"); //글자색을 녹색으로 변경
				// fprintf(stderr, "server>");	 //커서 출력
			}
		}

	} // end of for(;;)

	close(fd);
	return 0;
}

void QuitOnError(char *msg) // error func
{
	perror(msg);
	exit(1);
}

void *ManageSystem()
{ //명령어를 처리할 스레드

	int i;

	printf("[System:/help, /num_user, /num_chat, /ip_list]\n");

	while (1)
	{
		char bufmsg[MAX_CHAT + 1];

		fprintf(stderr, "\033[1;32m");	//글자색을 녹색으로 변경
		printf("server>");				//커서 출력
		fgets(bufmsg, MAX_CHAT, stdin); //명령어 입력

		if (!strcmp(bufmsg, "\n")) // 엔터 무시
			continue;
		else if (!strcmp(bufmsg, "/help\n"))
			printf("help, num_user, num_chat, ip_list\n");
		else if (!strcmp(bufmsg, "/num_user\n"))
			printf("현재 참가자 수 = %d\n", num_user);
		else if (!strcmp(bufmsg, "/num_chat\n"))
			printf("전체 대화의 수 = %d\n", num_chat);
		else if (!strcmp(bufmsg, "/ip_list\n"))
			for (i = 0; i < num_user; i++)
				printf("%s\n", ip_list[i]);
		else
			printf("해당 명령어가 없습니다. /help를 참조하세요.\n");
	}
}

int ListenClients(int host, int port, int backlog)
{

	int serv_sock;
	if ((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket() fail");
		exit(1);
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(host);

	if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("bind() fail");
		exit(1);
	}

	listen(serv_sock, backlog);

	return serv_sock;
}

void AddClient(int sock, struct sockaddr_in *new_sockaddr)
{

	char buf[20];

	inet_ntop(AF_INET, &new_sockaddr->sin_addr, buf, sizeof(buf));

	fprintf(stderr, "\033[1;0G"); // move to column position
	// write(1, "\033[0G", 4);
	fprintf(stderr, "\033[33m");	 // printf Yellow text
	printf("new client: %s\n", buf); //ip출력

	clnt_socks[num_user] = sock; // 채팅 클라이언트 목록에 추가
	num_user++;					 //유저 수 증가

	strcpy(ip_list[num_user], buf);
}

void RemoveClient(int sock_index)
{
	close(clnt_socks[sock_index]);

	if (sock_index != num_user - 1)
	{ //저장된 리스트 재배열
		clnt_socks[sock_index] = clnt_socks[num_user - 1];
		strcpy(ip_list[sock_index], ip_list[num_user - 1]);
	}

	num_user--; //유저 수 감소

	c_tm = time(NULL); //현재 시간을 받아옴
	tm = *localtime(&c_tm);

	fprintf(stderr, "\033[1;0G"); // move to column position
	//write(1, "\033[0G", 4);		//커서의 X좌표를 0으로 이동
	fprintf(stderr, "\033[33m"); //글자색을 노란색으로 변경
	printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
	printf("서버 접속자 1명 탈퇴. 현재 참가자 수 = %d\n", num_user);
	fprintf(stderr, "\033[32m");  //글자색을 녹색으로 변경
	fprintf(stderr, "server>\n"); //커서 출력
}

int GetMaxSockNum(int listen_sock)
{
	// Minimum 소켓번호는 가정 먼저 생성된 lstn_sock
	int max = listen_sock;
	int i;
	for (i = 0; i < num_user; i++)
		if (clnt_socks[i] > max)
			max = clnt_socks[i];

	return max;
}

void database()
{
	if (NULL == mysql_init(&conn)) // 초기화 함수
	{
		printError(__LINE__, conn);
	}
	else
	{
		printSuccess(__LINE__, conn);
	}

	if (NULL == (connection = mysql_real_connect(&conn, DB_SERV, DB_USER, DB_PSWD, DB_NAME, DB_PORT, DB_SOCK, 0)))
	{
		printError(__LINE__, conn);
	}
	else
	{
		printSuccess(__LINE__, conn);
		sql_result = mysql_store_result(connection); //쿼리에 대한 결과를 row에 저장
	}
}

void printError(int line_num, MYSQL conn) // print error func
{
	sprintf(checking_msg, " 에러 %4d - %s\n", mysql_errno(&conn), mysql_error(&conn));
	printf("\n[파일명-%s:라인-%03d] ", __FILE__, line_num);
	printf("%s\n", checking_msg);
	exit(1);
}

void printSuccess(int line_num, MYSQL conn) // print success func
{
	sprintf(checking_msg, " 성공 %4d - %s\n", mysql_errno(&conn), mysql_error(&conn));
	printf("\n[파일명-%s:라인-%03d] ", __FILE__, line_num);
	printf("%s\n", checking_msg);
}

void endDB()
{
	mysql_close(&conn);
}

void updateMemberDB()
{
	sprintf(query, "INSERT INTO `0627QTArduino`.memberlist(email, pw, nickname, clnt_time, serv_time) VALUES('%s', '%s', '%s', '%s', now())", sArr[1], sArr[2], sArr[3], sArr[4]);
	if (mysql_query(connection, query)) // mysql_query()는 SQL 성공시 0, 실패시 1 반환
	{
		printError(__LINE__, conn);
	}
	else
	{
		printSuccess(__LINE__, conn);
	}
	//mysql_free_result(sql_result);
}

void loginCustomerDB()
{

	sprintf(query, "INSERT INTO `0627QTArduino`.customoerLog(email, pw, nickname, clnt_time, serv_time) VALUES('%s', '%s', '%s', '%s', now())", sArr[1], sArr[2], sArr[3], sArr[4]);
	if (mysql_query(connection, query)) // mysql_query()는 SQL 성공시 0, 실패시 1 반환
	{
		printError(__LINE__, conn);
	}
	else
	{
		printSuccess(__LINE__, conn);
	}
	//mysql_free_result(sql_result);
}

void loginManagerDB()
{

	sprintf(query, "INSERT INTO `0627QTArduino`.managerLog(email, pw, nickname, clnt_time, serv_time) VALUES('%s', '%s', '%s', '%s', now())", sArr[1], sArr[2], sArr[3], sArr[4]);
	if (mysql_query(connection, query)) // mysql_query()는 SQL 성공시 0, 실패시 1 반환
	{
		printError(__LINE__, conn);
	}
	else
	{
		printSuccess(__LINE__, conn);
	}
	//mysql_free_result(sql_result);
}

void searchLogDB()
{

	printf("\nDB 검색 시작 \n");
	sprintf(query, "INSERT INTO `0627QTArduino`.searchLog(email, pw, nickname, clnt_time, serv_time) VALUES('%s', '%s', '%s', '%s', now())", sArr[1], sArr[2], sArr[3], sArr[4]);

	printf("\nDB 검색 중\n");
	if (mysql_query(connection, query)) // mysql_query()는 SQL 성공시 0, 실패시 1 반환
	{
		printError(__LINE__, conn);
	}
	else
	{
		printSuccess(__LINE__, conn);
	}
	printf("\nDB 검색 완료\n");
	// mysql_free_result(sql_result);
}

void sendDB()
{
	printf("\nDB 검색 전송 시작\n");

	sprintf(query, "SELECT * FROM `0627QTArduino`.memberlist");

	if (mysql_query(connection, query)) // mysql_query()는 SQL 성공시 0, 실패시 1 반환
	{
		printError(__LINE__, conn);
	}
	else
	{
		printSuccess(__LINE__, conn);
	}
	printf("\nDB 검색 전송 중\n");
	sql_result = mysql_store_result(connection);
	while (NULL != (sql_row = mysql_fetch_row(sql_result)))
	{
		sprintf(buf, "%s %s %s %s %s \n", sql_row[0], sql_row[1], sql_row[2], sql_row[3], sql_row[4]);
		//printf("%s %s %s %s %s \n", sql_row[0], sql_row[1], sql_row[2], sql_row[3], sql_row[4]);
		// 모든 채팅 참가자에게 메시지 방송
		for (int j = 0; j < num_user; j++)
			send(clnt_socks[j], buf, strlen(buf), 0);
	}
	printf("DB 검색 전송 완료\n");

	//mysql_free_result(sql_result);
}

void LedOn()
{
	printf("1\n");
	sprintf(query, "INSERT INTO `0627QTArduino`.usageLog(email, pw, nickname, clnt_time, serv_time, device) VALUES('%s', '%s', '%s', '%s', now(), '%s')", sArr[1], sArr[2], sArr[3], sArr[4], sArr[5]);
	if (mysql_query(connection, query)) // mysql_query()는 SQL 성공시 0, 실패시 1 반환
	{
		printError(__LINE__, conn);
	}
	else
	{
		printSuccess(__LINE__, conn);
	}
	printf("1\n");
	if (flag == 0)
	{

		printf("LED ON\n");
		write(fd, "1", 1);
		flag = 1;
	}
	else
	{

		printf("LED OFF\n");
		write(fd, "2", 1);
		flag = 0;
	}
}

void SpeakerOn()
{
	sprintf(query, "INSERT INTO `0627QTArduino`.usageLog(email, pw, nickname, clnt_time, serv_time, device) VALUES('%s', '%s', '%s', '%s', now(), '%s')", sArr[1], sArr[2], sArr[3], sArr[4], sArr[5]);
	if (mysql_query(connection, query)) // mysql_query()는 SQL 성공시 0, 실패시 1 반환
	{
		printError(__LINE__, conn);
	}
	else
	{
		printSuccess(__LINE__, conn);
	}
	if (flag == 0)
	{

		printf("SPEAKER ON\n");
		write(fd, "3", 1);
		flag = 1;
	}
	else
	{

		printf("SPEAKER OFF\n");
		write(fd, "4", 1);
		flag = 0;
	}
}

void LcdOn()
{
	sprintf(query, "INSERT INTO `0627QTArduino`.usageLog(email, pw, nickname, clnt_time, serv_time, device) VALUES('%s', '%s', '%s', '%s', now(), '%s')", sArr[1], sArr[2], sArr[3], sArr[4], sArr[5]);
	if (mysql_query(connection, query)) // mysql_query()는 SQL 성공시 0, 실패시 1 반환
	{
		printError(__LINE__, conn);
	}
	else
	{
		printSuccess(__LINE__, conn);
	}

	if (flag == 0)
	{

		printf("LCD ON\n");
		write(fd, "5", 1);
		flag = 1;
	}
	else
	{

		printf("LCD OFF\n");
		write(fd, "6", 1);
		flag = 0;
	}
}