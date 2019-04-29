#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <signal.h>
#include <fstream>
#include <iterator>
#include <NTL/ZZ.h>

#include "src/constants.hpp"
#include "src/vss.hpp"
#include "src/communicate.hpp"
#include "src/io.hpp"

using namespace NTL;
using namespace std;

int PORTNO[4] = {9000, 9001, 9002, 9003};

// Helper structure to send arguments to thread_func
struct ThreadArgs {
    int sd;
    FeldmanVSS vss;
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

FeldmanVSS get_shares(string secret_file) {
    ifstream fin(secret_file);
    istream_iterator<uchar> start(fin), end;
    vector<uchar> secret(start, end);
    secret.resize(SECRET_LEN, 0); // Resize secret to SECRET_LEN

    cout << "Secret: ";
    for(size_t i = 0; i < secret.size(); ++i) {
        cout << secret[i];
    }
    cout << "\n\n";

    cout << "Splitting secret.\n\n";
    FeldmanVSS v = FeldmanVSS::split(secret);

    return v;
}

void* start_server(void *args) {
    ThreadArgs temp = *((ThreadArgs*) args);
    int sd = temp.sd;
    FeldmanVSS vss = temp.vss;

    Share share;
    share.set_commits(vss.commits);
    share.set_share(vss.shares[0]);
    share.set_cipher(vss.cipher);
    share.xval = 1;

    while(1) {
        int nsd = accept(sd, NULL, NULL);

        Request req;
        if (read(nsd, &req, sizeof(req)) == -1) {
            syserr(AT);
        }

        switch (req.type) {
        case 1:
            cout << endl << endl << "===== Reconstruct Request =====\n";
            cout << "Shares being requested by party: " << req.party << endl;

            if (write(nsd, &share, sizeof(share)) == -1) {
                syserr(AT);
            }

            cout << "===== Reconstruct Request =====\n\n";
            break;

        case 2:
            cout << endl << endl << "===== Share Request =====\n";
            cout << "Party " << req.party << " requesting for its shares.\n";

            Share pshare;
            pshare.set_commits(vss.commits);
            pshare.set_share(vss.shares[req.party]);
            pshare.set_cipher(vss.cipher);
            pshare.xval = req.party + 1;

            if (write(nsd, &pshare, sizeof(pshare)) == -1) {
                syserr(AT);
            }

            cout << "===== Share Request =====\n\n";
            break;
        }
    }
}

void compute_secret(Share share) {
    int party;

    cout << "\n===== Computing Secret =====\n";
    cout << "Party to request: ";
    cin >> party;
    if (party > 3) party = 3; // only 3 parties
    if (party < 1) party = 1; // party cannot be dealer itself or negative

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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <secret_file>\n", argv[0]);
        return 0;
    }

    string secret_file(argv[1]);

    // Load prime and generator values
    setup();

    cout << "===== Init =====\n";
    cout << "Prime: " << FeldmanVSS::get_q() << endl;
    cout << "Generator: " << FeldmanVSS::get_g() << endl;
    cout << "===== Init =====\n";
    cout << endl;

    // Create socket for listening to incoming connections
    int sd = create_server_socket(PORTNO[0]);
    if (sd == -1) {
        return 0;
    }

    // Generate shares for secret stored in secret_file
    FeldmanVSS vss = get_shares(secret_file);
    cout << "===== VSS Output =====\n";
    vss.print();
    cout << "===== VSS Output =====\n";
    cout << endl;

    // Seperating dealer's share from rest
    Share dealer_share;
    dealer_share.set_commits(vss.commits);
    dealer_share.set_share(vss.shares[0]);
    dealer_share.set_cipher(vss.cipher);
    dealer_share.xval = 1;

    // Arguments to start_server
    ThreadArgs args = {.sd = sd, .vss = vss};
    pthread_t tid;
    pthread_create(&tid, NULL, &start_server, &args);

    // Show menu in foreground
    show_menu(dealer_share);

    // Kill server thread
    pthread_kill(tid, SIGKILL);
    pthread_join(tid, NULL);

    // Close server socket
    if (close(sd) == -1) {
        syserr(AT);
    }

    return 0;
}
