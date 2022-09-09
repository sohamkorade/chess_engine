#include <gtk/gtk.h>

#include <unordered_map>

#include "game.hpp"

Game g;
bool board_flipped = false;
GtkWidget *squares[64], *chess_board_grid, *statusbar, *move_label;
GtkEntryBuffer *fen_entry_buffer;
int sel_sq = -1, prom_sq = -1;
vector<Move> legals;
unordered_map<int, Move> promotions;

GtkWidget *white_human, *white_randommover, *white_ai, *black_human,
    *black_randommover, *black_ai;

void msg(string s) {
  gtk_statusbar_pop(GTK_STATUSBAR(statusbar), 0);
  gtk_statusbar_push(GTK_STATUSBAR(statusbar), 0, s.c_str());
}

void display_piece(int sq, char piece) {
  // const char piece_str[] = {piece, '\0'};
  // gtk_button_set_label(GTK_BUTTON(squares[sq]), piece_str);
  gtk_widget_set_name(squares[sq], "");
  if (piece == '.')
    gtk_widget_set_name(squares[sq], "");
  else
    gtk_widget_set_name(squares[sq], string("piece_").append(1, piece).c_str());
}

void generate_move_hints(unordered_set<int> &valid_sqs,
                         unordered_set<int> &valid_capture_sqs) {
  legals = g.board.generate_legal_moves();
  for (auto &move : legals)
    if (move.from == sel_sq)
      if (g.board[move.to] != '.')
        valid_capture_sqs.insert(board_flipped ? 63 - move.to : move.to);
      else
        valid_sqs.insert(board_flipped ? 63 - move.to : move.to);
  if (valid_sqs.size() == 0 && valid_capture_sqs.size() == 0) sel_sq = -1;
}

void update_board() {
  unordered_set<int> valid_sqs, valid_capture_sqs;
  if (~sel_sq && g.result == Undecided)
    generate_move_hints(valid_sqs, valid_capture_sqs);
  string &board = g.board.board;
  if (board_flipped) {
    reverse(board.begin(), board.end());
    if (~sel_sq) sel_sq = 63 - sel_sq;
  }
  int last_from = -1, last_to = -1;
  if (g.ply > 0) {
    Move last = g.movelist[g.ply - 1];
    last_from = last.from;
    last_to = last.to;
    if (board_flipped) {
      last_from = 63 - last_from;
      last_to = 63 - last_to;
    }
    msg("Moved " + g.board.to_san(last));
  } else
    msg("");
  for (int i = 0; i < 64; i++) {
    display_piece(i, board[i]);
    for (auto &c : {"selected_sq", "valid_sq", "valid_capture_sq", "check_sq"})
      gtk_widget_remove_css_class(squares[i], c);
    if (i == sel_sq || i == last_from || i == last_to)
      gtk_widget_add_css_class(squares[i], "selected_sq");
    if (valid_sqs.count(i)) gtk_widget_add_css_class(squares[i], "valid_sq");
    if (valid_capture_sqs.count(i))
      gtk_widget_add_css_class(squares[i], "valid_capture_sq");
  }
  int K_pos = board.find(g.board.turn == White ? 'K' : 'k');
  if (~K_pos && g.board.is_in_check(g.board.turn))
    gtk_widget_add_css_class(squares[K_pos], "check_sq");

  if (board_flipped) {
    reverse(board.begin(), board.end());
    if (~sel_sq) sel_sq = 63 - sel_sq;
  }
}

void update_gui() {
  update_board();

  string fen = g.board.to_fen();
  gtk_entry_buffer_set_text(fen_entry_buffer, fen.c_str(), fen.length());
  gtk_label_set_text(
      GTK_LABEL(move_label),
      ("Move: " + to_string(g.ply / 2 + 1) + " " + g.get_result_str(g.result))
          .c_str());

  g.result = g.get_result();
  if (g.result == Draw) cout << g.ply / 2 + 1 << endl;
  for (auto &x : g.white_alive) cout << x;
  cout << " vs ";
  for (auto &x : g.black_alive) cout << x;
  cout << g.result << endl;
}

void first_click() { g.seek(0), update_gui(); }
void prev_click() { g.prev(), update_gui(); }
void next_click() { g.next(), update_gui(); }
void last_click() { g.seek(g.end), update_gui(); }
void flip_click() {
  board_flipped = !board_flipped;
  if (~sel_sq) sel_sq = 63 - sel_sq;
  update_gui();
}
void lichess_click() {
  string fen = g.board.to_fen();
  replace(fen.begin(), fen.end(), ' ', '_');
  cout << "https://lichess.org/analysis/" << fen << endl;
  gtk_show_uri(NULL, ("https://lichess.org/analysis/" + fen).c_str(),
               GDK_CURRENT_TIME);
}
void newgame_click() {
  g.new_game();
  update_gui();
}
void fen_apply_click() {
  if (g.load_fen(gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(fen_entry_buffer))))
    msg("FEN loaded.");
  else
    msg("Bad FEN.");
  update_gui();
}

void make_move(Move m) {
  if (g.make_move(m)) {
    if (g.board.turn == White) {
      if (gtk_check_button_get_active(GTK_CHECK_BUTTON(white_randommover))) {
        make_move(g.random_move());
      } else if (gtk_check_button_get_active(GTK_CHECK_BUTTON(white_ai))) {
        make_move(g.ai_move());
      }
    } else if (g.board.turn == Black) {
      if (gtk_check_button_get_active(GTK_CHECK_BUTTON(black_randommover))) {
        make_move(g.random_move());
      } else if (gtk_check_button_get_active(GTK_CHECK_BUTTON(black_ai))) {
        make_move(g.ai_move());
      }
    }
  }
  update_gui();
}

void square_click(GtkWidget *widget, gpointer data) {
  if (g.result != Undecided) return;
  int sq = GPOINTER_TO_INT(data);
  if (board_flipped) sq = 63 - sq;
  if (~prom_sq) {
    for (auto &x : promotions) {
      if (x.first == sq) make_move(x.second);  // clicked on promotion piece
      gtk_widget_remove_css_class(squares[x.first], "promotion");
    }
    gtk_widget_remove_css_class(chess_board_grid, "promotion");
    promotions.clear();
    prom_sq = -1;
  }
  if (~sel_sq) {
    vector<Move> valids;
    for (auto &move : legals)
      if (move.from == sel_sq && move.to == sq) valids.push_back(move);
    sel_sq = -1;
    if (valids.size() == 1)
      make_move(valids.front());
    else if (valids.size() > 1) {  // move is a promotion
      prom_sq = valids.front().to;
      Direction d = prom_sq / 8 == 0 ? S : N;
      for (int i = 0; i < valids.size(); i++) {
        int temp_sq = prom_sq + d * i;
        promotions.insert({temp_sq, valids[i]});
        gtk_widget_add_css_class(squares[temp_sq], "promotion");
        display_piece(temp_sq, valids[i].promotion);
      }
      gtk_widget_add_css_class(chess_board_grid, "promotion");
      return;
    } else  // illegal move
      sel_sq = sq;
  } else
    sel_sq = sq;

  update_gui();
}

GtkWidget *chess_board() {
  chess_board_grid = gtk_grid_new();
  for (int i = 0; i < 64; i++) {
    squares[i] = gtk_button_new();
    g_signal_connect(squares[i], "clicked", G_CALLBACK(square_click),
                     GINT_TO_POINTER(i));
    gtk_button_set_has_frame(GTK_BUTTON(squares[i]), false);
    gtk_widget_add_css_class(squares[i], "piece");
    gtk_grid_attach(GTK_GRID(chess_board_grid), squares[i], i % 8, i / 8, 1, 1);
  }
  gtk_grid_set_column_homogeneous(GTK_GRID(chess_board_grid), true);
  gtk_grid_set_row_homogeneous(GTK_GRID(chess_board_grid), true);
  gtk_widget_set_size_request(chess_board_grid, 600, 600);

  gtk_widget_set_name(chess_board_grid, "board");
  return chess_board_grid;
}

pair<GtkWidget *, GtkWidget *> create_player_chooser() {
  white_human = gtk_check_button_new_with_label("Human");
  white_randommover = gtk_check_button_new_with_label("Random mover");
  white_ai = gtk_check_button_new_with_label("AI");
  gtk_check_button_set_group(GTK_CHECK_BUTTON(white_randommover),
                             GTK_CHECK_BUTTON(white_human));
  gtk_check_button_set_group(GTK_CHECK_BUTTON(white_ai),
                             GTK_CHECK_BUTTON(white_human));
  GtkWidget *white_frame = gtk_frame_new("White"),
            *white_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_append(GTK_BOX(white_box), white_human);
  gtk_box_append(GTK_BOX(white_box), white_randommover);
  gtk_box_append(GTK_BOX(white_box), white_ai);
  gtk_frame_set_child(GTK_FRAME(white_frame), white_box);
  gtk_check_button_set_active(GTK_CHECK_BUTTON(white_human), true);

  black_human = gtk_check_button_new_with_label("Human");
  black_randommover = gtk_check_button_new_with_label("Random mover");
  black_ai = gtk_check_button_new_with_label("AI");
  gtk_check_button_set_group(GTK_CHECK_BUTTON(black_randommover),
                             GTK_CHECK_BUTTON(black_human));
  gtk_check_button_set_group(GTK_CHECK_BUTTON(black_ai),
                             GTK_CHECK_BUTTON(black_human));
  GtkWidget *black_frame = gtk_frame_new("Black"),
            *black_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_append(GTK_BOX(black_box), black_human);
  gtk_box_append(GTK_BOX(black_box), black_randommover);
  gtk_box_append(GTK_BOX(black_box), black_ai);
  gtk_frame_set_child(GTK_FRAME(black_frame), black_box);
  gtk_check_button_set_active(GTK_CHECK_BUTTON(black_randommover), true);

  return {white_frame, black_frame};
}

GtkWidget *navigation_buttons() {
  GtkWidget *grid = gtk_grid_new();
  move_label = gtk_label_new("Move 1");
  GtkWidget *first = gtk_button_new_from_icon_name("go-first");
  GtkWidget *prev = gtk_button_new_from_icon_name("go-previous");
  GtkWidget *next = gtk_button_new_from_icon_name("go-next");
  GtkWidget *last = gtk_button_new_from_icon_name("go-last");
  GtkWidget *flip = gtk_button_new_with_label("Flip board");
  GtkWidget *lichess = gtk_button_new_with_label("Open in lichess");
  GtkWidget *newgame = gtk_button_new_with_label("New game");

  gtk_grid_attach(GTK_GRID(grid), move_label, 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), first, 1, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), prev, 2, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), next, 3, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), last, 4, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), flip, 1, 3, 4, 1);
  gtk_grid_attach(GTK_GRID(grid), lichess, 1, 4, 4, 1);
  gtk_grid_attach(GTK_GRID(grid), newgame, 1, 5, 4, 1);

  auto [white_frame, black_frame] = create_player_chooser();
  gtk_grid_attach(GTK_GRID(grid), white_frame, 1, 6, 4, 1);
  gtk_grid_attach(GTK_GRID(grid), black_frame, 1, 7, 4, 1);

  g_signal_connect(first, "clicked", G_CALLBACK(first_click), NULL);
  g_signal_connect(prev, "clicked", G_CALLBACK(prev_click), NULL);
  g_signal_connect(next, "clicked", G_CALLBACK(next_click), NULL);
  g_signal_connect(last, "clicked", G_CALLBACK(last_click), NULL);
  g_signal_connect(flip, "clicked", G_CALLBACK(flip_click), NULL);
  g_signal_connect(lichess, "clicked", G_CALLBACK(lichess_click), NULL);
  g_signal_connect(newgame, "clicked", G_CALLBACK(newgame_click), NULL);

  gtk_widget_set_margin_start(grid, 5);

  return grid;
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid), chess_board(), 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), navigation_buttons(), 2, 1, 1, 1);
  GtkWidget *fen_entry = gtk_entry_new();
  fen_entry_buffer = gtk_entry_buffer_new("", 0);
  gtk_entry_set_buffer(GTK_ENTRY(fen_entry), fen_entry_buffer);
  // gtk_entry_set_placeholder_text(GTK_ENTRY(fen_entry), "FEN");
  GtkWidget *fen_expander = gtk_expander_new("FEN");
  GtkWidget *fen_apply = gtk_button_new_from_icon_name("dialog-ok");
  GtkWidget *fen_grid = gtk_grid_new();
  gtk_grid_attach(GTK_GRID(fen_grid), fen_entry, 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(fen_grid), fen_apply, 2, 1, 1, 1);
  gtk_expander_set_child(GTK_EXPANDER(fen_expander), fen_grid);
  gtk_grid_attach(GTK_GRID(grid), fen_expander, 1, 2, 2, 1);
  g_signal_connect(fen_apply, "clicked", G_CALLBACK(fen_apply_click), NULL);
  statusbar = gtk_statusbar_new();
  gtk_grid_attach(GTK_GRID(grid), statusbar, 1, 3, 2, 1);

  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Chess by Soham");
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
  gtk_window_set_child(GTK_WINDOW(window), grid);

  GtkCssProvider *cssProvider = gtk_css_provider_new();
  gtk_css_provider_load_from_path(cssProvider, "interface/style.css");
  gtk_style_context_add_provider_for_display(gtk_widget_get_display(window),
                                             GTK_STYLE_PROVIDER(cssProvider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);

  gtk_window_present(GTK_WINDOW(window));
  // g.board.load_fen("8/2P5/2N3k1/8/8/2B5/2p2K2/8 w - - 0 1");
  update_gui();
}

int main(int argc, char **argv) {
  srand(time(0));
  GtkApplication *app =
      gtk_application_new("org.soham.chess", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}