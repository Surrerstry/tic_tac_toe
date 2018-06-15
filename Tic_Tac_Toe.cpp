/*
g++ -std=c++17 Tic_Tac_Toe.cpp -o Tic_Tac_Toe

Author: Surrerstry
Version: 1.0
*/

#include <iostream>
#include <cctype>
#include <vector>
#include <sstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_MSG_LEN 3
#define MAX_CONNECTION 1

using namespace std;

string wait_on_data(const int socket_, socklen_t len, sockaddr_in client);
int send_data(string data_to_send);

int LocalServerPort = 8888;
string LocalServerIP = "127.0.0.1";

int ForeignServerPort;
string ForeignServerAddress;

class Game {
	private:

		string last_error = "";

		int current_player = 1;

		vector<vector<int>> board = {
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
		};

		void switch_player() {
			if(current_player == 1)
				current_player = 2;
			else
				current_player = 1;
		}

		int save_move(string move) {
			int x, y;

			switch(move[0]) {
				case '1':
					x = 0;
					break;
				case '2':
					x = 1;
					break;
				case '3':
					x = 2;
					break;
				default:
					last_error = "Error: Incorrect column.";
					return 11;
			}

			switch(move[1]) {
				case 'a':
					y = 0;
					break;
				case 'b':
					y = 1;
					break;
				case 'c':
					y = 2;
					break;
				default:
					last_error = "Error: Incorrect row.";
					return 11;
			}

			if(board[y][x] != 0) {
				last_error = "Error: Field matched already.";
				return 11;
			}

			board[y][x] = current_player;
			
			last_position_x = x;
			last_position_y = y;
			
			switch_player();
		}

	public:

		int last_position_x;
		int last_position_y;

		char user_letter = ' ';

		void draw_board() {

		cout << endl << "Your letter: " << user_letter;
		if(user_letter == 'X')
			cout << " (First Player)" << endl;
		else 
			cout << " (Second Player)" << endl;
		cout << endl << "      1 2 3";
		cout << endl << "     -------" << endl << "   ";
		int i = 0;
		for(auto line: board) {
			for(auto element: line) {
				if(i == 0)
					cout << "A ";
				else if(i == 3)
					cout << "\b\bB ";
				else if(i == 6)
					cout << "\b\bC ";
	
				if(element == 0) 
					cout << "|" << " ";
				else if(element == 1) 
					cout << "|" << "X";
				else if(element == 2)
					cout << "|" << "Y";
				i++;
			}
			cout << "|" << endl << "     ";
		}
		cout << "-------" << endl << endl;
		}

		int make_move() {
			// return 10 means that move have to be repeated

			string move;

			while (true) {
				if(last_error.length() != 0) {
					cout << last_error << endl;
					last_error = "";
				}

				cout << "Type move: ";
				getline(cin, move, '\n');
	
				if(move.length() != 2) {
					last_error = "Error: Incorrect length of command. Should be only one digit and one letter.";
					return 10;
				}

				if(!isdigit(move[0])) {
					last_error = "Error: First sign should be a number.";
					return 10;
				}
				if(!isalpha(move[1])) {
					last_error = "Error: Second sign should be a letter.";
					return 10;
				}
				move[1] = tolower(move[1]);

				if(save_move(move) == 11) {
					return 10;
				}

				break;
			}

		return 0;
		}

		int check_situation() {
			/*
			0 - No one win yet
			1 - First player won
			2 - Second player won
			3 - Tie
			*/

			for(int player=1; player < 3; player++) {

				// Check horizontally
				if(board[0][0] == player && board[0][1] == player && board[0][2] == player) {
					return player;
				}
				if(board[1][0] == player && board[1][1] == player && board[1][2] == player) {
					return player;
				}
				if(board[2][0] == player && board[2][1] == player && board[2][2] == player) {
					return player;
				}

				// Check vertically
				if(board[0][0] == player && board[1][0] == player && board[2][0] == player) {
					return player;
				}
				if(board[0][1] == player && board[1][1] == player && board[2][1] == player) {
					return player;
				}
				if(board[0][2] == player && board[1][2] == player && board[2][2] == player) {
					return player;
				}

				// Check diagonally
				if(board[0][0] == player && board[1][1] == player && board[2][2] == player) {
					return player;
				}
				if(board[0][2] == player && board[1][1] == player && board[2][0] == player) {
					return player;
				}
			}

			// Check if there are any moves yet
			int sum = 0;
			for(auto row: board)
				for(auto field: row)
					if(field != 0)
						sum++;

			if(sum == 9)
				return 3;

			return 0;
		}

		void do_random_move() {

			vector<vector<int>> available_moves;

			// gather available moves
			for(int i=0; i<board.size(); i++) {
				for(int j=0; j<board[i].size(); j++) {
					if(board[i][j] == 0) {
						available_moves.push_back({i, j});
					}
				}
			}

			// choose random move from available ones
			int choose = rand() % available_moves.size();

			// do move
			board[available_moves[choose][0]][available_moves[choose][1]] = current_player;

			switch_player();
		
			return;
		}

		void second_player_move(const int socket_, socklen_t len, sockaddr_in client) {
			string data_from_second_user;
			data_from_second_user = wait_on_data(socket_, len, client);

			if(data_from_second_user.find("ERROR") != string::npos) {
				cerr << "Error in connection! (" << data_from_second_user << ")" << endl;
				exit(2);
			}

			// Expects two digits number
			int x, y;
			stringstream tmp_stream;
			
			tmp_stream << data_from_second_user[0];
			tmp_stream >> x;
			tmp_stream.str("");
			tmp_stream.clear();

			tmp_stream << data_from_second_user[1];
			tmp_stream >> y;

			board[x][y] = current_player;

			switch_player();

		}
};


int main() {

	system("clear");
	srand(time(NULL));

	char human_or_machine;

	while(true) {
		cout << "\nDo you want to play with human(1), or computer(2)?: ";
		human_or_machine = cin.get();

		if(human_or_machine == '1' || human_or_machine == '2') {
			cin.ignore();
			break;
		}
	}

	if(human_or_machine == '1') {
	
	cout << "\nType your local IP address: ";
	getline(cin, LocalServerIP, '\n');
	cout << "Type your local port number: ";
	cin >> LocalServerPort;
	cin.ignore();

	cout << "\nType IP address of Second Player: ";
	getline(cin, ForeignServerAddress, '\n');
	cout << "Type Port of Second Player: ";
	cin >> ForeignServerPort;
	cin.ignore();

	}

	struct sockaddr_in local_server_struct = {
        .sin_family = AF_INET,
        .sin_port = htons(LocalServerPort)
    };

	if(inet_pton(AF_INET, LocalServerIP.c_str(), &local_server_struct.sin_addr) <= 0) {
        cerr << "inet_pton() ERROR" << endl;
        exit(1);
    }

    const int socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_ < 0) {
        cerr << "socket() ERROR" << endl;
        exit(1);
    }

    socklen_t len = sizeof(local_server_struct);
    if(bind(socket_, (struct sockaddr *)&local_server_struct, len) < 0) {
        cerr << "bind() ERROR" << endl;
        exit(1);
    }
   
    if(listen(socket_, MAX_CONNECTION) < 0) {
        cerr << "listen() ERROR" << endl;
    }
     
    struct sockaddr_in client = { };


	while(true) {

		Game game;

		int current_situation;
		char which_move;

		while(true) {
			if(human_or_machine == '1')
				cout << "\nWho first, You(1) or Other Player(2)?: ";
			else
				cout << "\nWho first, You(1) or Computer(2)?: ";
			which_move = cin.get();
	
				if(which_move == '1' || which_move == '2') {
					cin.ignore();
					break;
				}
		}
			
		while (true) {
			system("clear");
			
			if(which_move == '1') {
				if(game.user_letter == ' ')
					game.user_letter = 'X';
				game.draw_board();
				if(game.make_move() == 10)
					continue;
				if(human_or_machine == '1') {
					string x;
					string y;
					stringstream tmp_stream;
					
					tmp_stream << game.last_position_x;
					tmp_stream >> x;
					tmp_stream.str("");
					tmp_stream.clear();

					tmp_stream << game.last_position_y;
					tmp_stream >> y;

					send_data(y+x);
				}
				which_move = '2';
			} else {
				if(game.user_letter == ' ')
					game.user_letter = 'Y';
				game.draw_board();
				
				if(human_or_machine == '1') {
					game.second_player_move(socket_, len, client);
				} else
					game.do_random_move();

				which_move = '1';
			}

			current_situation = game.check_situation();

			if(current_situation != 0) {
				system("clear");
				game.draw_board();
				break;
				}
		}
	
		switch(current_situation) {
			case 1:
				cout << "First player won!" << endl;
				break;
			case 2:
				cout << "Second player won!" << endl;
				break;
			default:
				cout << "No one won, Tie!" << endl;
				break;
		}

	char play_again;
	
	cout << "\n    Play again? [Y/n]: ";
	play_again = cin.get();
	cin.ignore();

	if(play_again == 'N' || play_again == 'n')
		break;
	
	system("clear");

	}

	shutdown(socket_, SHUT_RDWR);

	return 0;
}

string wait_on_data(const int socket_, socklen_t len, sockaddr_in client) {

    cout << "Waiting for your opponent..." << endl;

    const int clientSocket = accept(socket_, (struct sockaddr*)&client, &len);
    if(clientSocket < 0)
       return "accept() ERROR";

    char buffer[MAX_MSG_LEN] = {};
    string buffer_to_res;
       
    if(recv(clientSocket, buffer, sizeof(buffer), 0) <= 0)
        return "recv() ERROR";

    shutdown(clientSocket, SHUT_RDWR);

    buffer_to_res = buffer;
    return buffer_to_res;
}

int send_data(string data_to_send) {

	struct sockaddr_in foreign_server_struct = {
        .sin_family = AF_INET,
        .sin_port = htons(ForeignServerPort)
    };

    int sock = 0;
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "Socket creation error" << endl;
        return -1;
    }
  
    if(inet_pton(AF_INET, ForeignServerAddress.c_str(), &foreign_server_struct.sin_addr) <= 0) {
        cerr << "Invalid address/ Address not supported" << endl;
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&foreign_server_struct, sizeof(foreign_server_struct)) < 0) {
        cerr << "Connection Failed" << endl;
        return -1;
    }

    send(sock, data_to_send.c_str(), data_to_send.length(), 0);
    shutdown(sock, SHUT_RDWR);

    return 0;
}


