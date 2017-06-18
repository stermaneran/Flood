/* This file contains all of the functions and associated variables */

extern int port;

//strtoint helper func
//recives the string(numbers in string format) , num of digites;
//converts str to int
int strtoint_n(char* str, int n)
{
    int place = 1;
    int ret = 0;
    int i;
    for (i = n-1; i >= 0; i--, place *= 10)
    {
        int c = str[i];
            if (c >= '0' && c <= '9')   ret += (c - '0') * place;
            else
            {
              printf("Invalid port!");
              return -1;
            }
    }
    //bad ports or max port num
    if(ret < 0 || ret > 65535)
    {
      printf("Invalid port! not in range!");

      return -1;
    }
    return ret;
}

//count digites in str then convert
int strtoint(char* str)
{
    char* temp = str;
    int n = 0;
    //count digites
    while (*temp != '\0')
    {
        n++;
        temp++;
    }
    int ret = strtoint_n(str, n);
    if(ret == -1)
    {
      printf("Error with port parsing!");
      exit(1);
    }
    return ret;
}

//genarate random port num 0-65535 with seed
int randomPort()
{
  time_t t;
  srand((unsigned) time(&t));
  port = (port + rand()) % 65535;
  int r = port;
  return r;
}

//calculate_checksum calculating
unsigned short calculate_checksum(unsigned short * paddress, int len)
{
	int nleft = len;
	int sum = 0;
	unsigned short * w = paddress;
	unsigned short answer = 0;

	while (nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1)
	{
		*((unsigned char *)&answer) = *((unsigned char *)w);
		sum += answer;
	}

	// add back carry outs from top 16 bits to low 16 bits
	sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
	sum += (sum >> 16);                 // add carry
	answer = ~sum;                      // truncate to 16 bits

	return answer;
}
