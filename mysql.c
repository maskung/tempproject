/* Simple C program that connects to MySQL Database server*/
#include <mysql.h>
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include "locking.h"

#define MAXTIMINGS 85
static int DHTPIN = 7;
static int dht22_dat[5] = {0,0,0,0,0};

float temp = 0;
float humid = 0;

static uint8_t sizecvt(const int read)
{
  /* digitalRead() and friends from wiringpi are defined as returning a value
  < 256. However, they are returned as int() types. This is a safety function */

  if (read > 255 || read < 0)
  {
    printf("Invalid data from wiringPi library\n");
    exit(EXIT_FAILURE);
  }
  return (uint8_t)read;
}

static int read_dht22_dat()
{
  uint8_t laststate = HIGH;
  uint8_t counter = 0;
  uint8_t j = 0, i;

  dht22_dat[0] = dht22_dat[1] = dht22_dat[2] = dht22_dat[3] = dht22_dat[4] = 0;

  // pull pin down for 18 milliseconds
  pinMode(DHTPIN, OUTPUT);
  digitalWrite(DHTPIN, HIGH);
  delay(10);
  digitalWrite(DHTPIN, LOW);
  delay(18);
  // then pull it up for 40 microseconds
  digitalWrite(DHTPIN, HIGH);
  delayMicroseconds(40); 
  // prepare to read the pin
  pinMode(DHTPIN, INPUT);

  // detect change and read data
  for ( i=0; i< MAXTIMINGS; i++) {
    counter = 0;
    while (sizecvt(digitalRead(DHTPIN)) == laststate) {
      counter++;
      delayMicroseconds(1);
      if (counter == 255) {
        break;
      }
    }
    laststate = sizecvt(digitalRead(DHTPIN));

    if (counter == 255) break;

    // ignore first 3 transitions
    if ((i >= 4) && (i%2 == 0)) {
      // shove each bit into the storage bytes
      dht22_dat[j/8] <<= 1;
      if (counter > 16)
        dht22_dat[j/8] |= 1;
      j++;
    }
  }

  // check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
  // print it out if data is good
  if ((j >= 40) && 
      (dht22_dat[4] == ((dht22_dat[0] + dht22_dat[1] + dht22_dat[2] + dht22_dat[3]) & 0xFF)) ) {
        float t, h;
        h = (float)dht22_dat[0] * 256 + (float)dht22_dat[1];
        h /= 10;
        t = (float)(dht22_dat[2] & 0x7F)* 256 + (float)dht22_dat[3];
        t /= 10.0;
        if ((dht22_dat[2] & 0x80) != 0)  t *= -1;


    //printf("Humidity = %.2f %% Temperature = %.2f *C \n", h, t );
    //put temperature and humidity to out side function 
    temp = t;
    humid = h;

    return 1;
  }
  else
  {
    printf("Data not good, skip\n");
    return 0;
  }
}


main() {

  int lockfd;
  int tries = 100;

   MYSQL *conn;
   MYSQL_RES *res;
   MYSQL_ROW row;

   char *server = "localhost";
   char *user = "root";
   char *password = "password"; /* set me first */
   char *database = "environment";
   char sql[255];

   conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, server,
         user, password, database, 0, NULL, 0)) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      exit(1);
   }

  if (wiringPiSetup () == -1)
    exit(EXIT_FAILURE) ;

  if (setuid(getuid()) < 0)
  {
    perror("Dropping privileges failed\n");
    exit(EXIT_FAILURE);
  }

  while (read_dht22_dat() == 0 && tries--) 
  {
     delay(1000); // wait 1sec to refresh
  }

  delay(1500);
  close_lockfile(lockfd);

  printf("Humidity = %.2f %% Temperature = %.2f *C \n", humid, temp );
  //sprintf(sql,"\n", humid, temp );

  sprintf(sql,"INSERT INTO `enviroment` (`id`, `temperature`, `humidity`, `created`) VALUES (NULL, '%.2f', '%.2f', NOW());", temp, humid);

   /* insert data to database */
   if (mysql_query(conn, sql)) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      exit(1);
   } 
   


   /* send SQL query */
   /*if (mysql_query(conn, "show tables")) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      exit(1);
   } */

   res = mysql_use_result(conn);

   /* output table name */
   /*printf("MySQL Tables in mysql database:\n");
   while ((row = mysql_fetch_row(res)) != NULL)
      printf("%s \n", row[0]);

   */

   /* close connection */
   mysql_free_result(res);
   mysql_close(conn);
}

