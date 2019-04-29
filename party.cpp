#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <signal.h>
#include <fstream>
#include <NTL/ZZ.h>

#include "src/constants.hpp"
#include "src/vss.hpp"
#include "src/communicate.hpp"
#include "src/io.hpp"

int PORTNO[4] = {9000, 9001, 9002, 9003};
int mynum;

// Helper structure to send arguments to thread_func
struct ThreadArgs {
    int sd;
    Share share;
};

int create_server_socket(int portno) {
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        syserr(AT);
        return -1;
    }

    int reuse = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const char*) &reuse, sizeof(reuse)) == -1) {
        syserr(AT);
    }

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(portno);

    if (::bind(sd, (struct sockaddr*) &serv, sizeof(serv)) < 0) {
        syserr(AT);

        if (close(sd) == -1) {
            syserr(AT);
        }

        return -1;
    }

    if (listen(sd, 5) == -1) {
        syserr(AT);

        if (close(sd) == -1) {
            syserr(AT);
        }

        return -1;
    }

    return sd;
}

int connect_to_party(int portno) {
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        syserr(AT);
        return -1;
    }

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(portno);

    if (connect(sd, (struct sockaddr*) &serv, sizeof(serv)) == -1) {
        syserr(AT);
        if (close(sd) == -1) {
            syserr(AT);
        }

        return -1;
    }

    return sd;
}

void setup() {
    Init i;
    i.load(INIT_FILE);

    FeldmanVSS::load(i.get_q(), i.get_g());
}

void* start_server(void *args) {
    ThreadArgs temp = *((ThreadArgs*) args);
    int sd = temp.sd;
    Share share = temp.share;

    while(1) {
        int nsd = accept(sd, NULL, NULL);

        Request req;
        if (read(nsd, &req, sizeof(req)) == -1) {
            syserr(AT);
        }

        cout << "\n\n===== Incoming Request =====\n";
        cout << "\nShares being requested by party: " << req.party << endl;

        if (write(nsd, &share, sizeof(share)) == -1) {
            syserr(AT);
        }

        cout << "===== Incoming Request =====\n\n";
    }
}

void compute_secret(Share share) {
    int party;

    cout << "\n===== Computing Secret =====\n";
    cout << "Party to request: ";
    cin >> party;
    if (party > 3) party = 3; // only 3 parties
    if (party < 0) party = 0;
    if (party == mynum) {
        if (mynum == 3) party -= 1;
        else party += 1;
    }

    int sd = connect_to_party(PORTNO[party]);

    Request req = {.party = 0, .type = 1};
    if (write(sd, &req, sizeof(req)) == -1) {
        syserr(AT);
    }

    cout << "\nRequest sent to party: " << party << endl;

    Share pshare;
    if (read(sd, &pshare, sizeof(pshare)) == -1) {
        syserr(AT);
    }

    if (close(sd) == -1) {
        syserr(AT);
    }

    cout << "\n<--- Received Share --->\n";
    pshare.print();
    cout << "<--- Received Share --->\n\n";
    cout << endl;

    bool ok = FeldmanVSS::verify(
        share.get_commits(),
        pshare.xval,
        pshare.get_share()
    );

    if (!ok) {
        cout << "Party is malicious. Not reconstructing.\n";
    } else {
        cout << "Verification Successful. Reconstructing.\n";

        vector<uchar> secret = FeldmanVSS::reconstruct(
            {share.xval, pshare.xval},
            {share.get_share(), pshare.get_share()},
            {share.get_cipher()}
        );

        cout << "Secret: ";
        for(uchar i : secret) {
            cout << i;
        }
        cout << endl;
    }

    cout << "\n===== Computing Secret =====\n";
}

void show_menu(Share share) {
    do {
        cout << endl;
        cout << "===== Menu =====\n";
        cout << "1: Compute secret\n";
        cout << "2: Exit\n";
        cout << "Option: ";
        int opt;
        cin >> opt;
        cout << endl;

        switch(opt) {
        case 1:
            compute_secret(share);
            break;

        case 2:
            return;

        default:
            cout << "Invalid choice.\n";
        }
    } while(true);
}

Share receive_share() {
    Share resp;
    Request req = {.party = mynum, .type = 2};
    int sd = connect_to_party(PORTNO[0]);

    if (write(sd, &req, sizeof(req)) == -1) {
        syserr(AT);
    }

    if (read(sd, &resp, sizeof(resp)) == -1) {
        syserr(AT);
    }

    if (close(sd) == -1) {
        syserr(AT);
    }

    cout << "\n===== Receive Share =====\n";
    resp.print();

    bool ok = FeldmanVSS::verify(
        resp.get_commits(),
        resp.xval,
        resp.get_share()
    );

    if (ok) {
        cout<<"\nShare verified successfully.\n";
    } else {
        cout<<"\nShare verification failed. Possible malicious dealer.\n";
    }
    cout << "===== Receive Share =====\n\n";

    return resp;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <party_num: [1,3]> <is_malicious: 0|1>\n", argv[0]);
        return 0;
    }

    // Convert command line arguments
    mynum = atoi(argv[1]);
    if (mynum < 1) mynum = 1;
    else if (mynum > 3) mynum = 3;

    int malicious = atoi(argv[2]);

    // Load prime and generator values
    setup();

    cout << "===== Init =====\n";
    cout << "Prime: " << FeldmanVSS::get_q() << endl;
    cout << "Generator: " << FeldmanVSS::get_g() << endl;
    cout << "===== Init =====\n";
    cout << endl;

    // Create socket to listen for incoming connections
    int sd = create_server_socket(PORTNO[mynum]);
    if (sd == -1) {
        return 0;
    }

    // Receive share from dealer
    Share share = receive_share();

    // If malicious transform share to incorrect values
    if (malicious) {
        cout << "Modifying share to random values since party is malicious\n";
        ZZ_p new_share = random_ZZ_p();
        share.set_share(new_share);

        cout << "<--- New Share --->\n";
        share.print();
        cout << "<--- New Share --->\n";
        cout << endl;
    }

    // Start server in new thread
    ThreadArgs args = {.sd = sd, .share = share};
    pthread_t tid;
    pthread_create(&tid, NULL, &start_server, &args);

    // Show menu in foreground
    show_menu(share);

    // Stop server
    pthread_kill(tid, SIGKILL);
    pthread_join(tid, NULL);

    // Close server socket
    if (close(sd) == -1) {
        syserr(AT);
    }

    return 0;
}
