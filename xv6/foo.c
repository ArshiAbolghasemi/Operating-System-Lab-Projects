#define NUM_FORKS 10
int main(int argc, char* argv[]){
    for (int i = 0; i < NUM_FORKS; i++){
        int pid = fork();
        if (pid == 0){
            sleep(4000);
            int x = 0;
            for (long long j = 0; j < 100000000000; j++){
                for (long long k = 0; k < 1000000000000; k++)
                x += k * 12 - j;
            }
            break;
        }
    }
    while(wait() != -1);
    exite();
}