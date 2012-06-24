/********************* MODIFICATIONS to py5.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/05/09 SE   New conditions: SAT, StrictSAT, SAT X Y (invented L.Salai sr.)
 **
 ** 2006/05/14 SE   New Condition: TakeMake (invented H.Laue)
 **
 ** 2006/06/28 SE   New condition: Masand (invented P.Petkov)
 **
 ** 2006/06/30 SE   New condition: BGL (invented P.Petkov)
 **
 ** 2006/07/30 SE   New condition: Schwarzschacher
 **
 ** 2007/01/28 SE   New condition: Annan Chess
 **
 ** 2007/05/01 SE   Bugfix: StrictSAT bug
 **
 ** 2007/05/04 SE   Bugfix: SAT + BlackMustCapture
 **
 ** 2007/05/04 SE   Bugfix: SAT + Ultraschachzwang
 **
 ** 2007/11/08 SE   New conditions: Vaulting kings (invented: J.G.Ingram)
 **                 Transmuting/Reflecting Ks now take optional piece list
 **                 turning them into vaulting types
 **
 ** 2007/12/26 SE   New piece: Reverse Pawn (for below but independent)
 **                 New condition: Protean Chess
 **                 New piece type: Protean man (invent A.H.Kniest?)
 **                 (Differs from Frankfurt chess in that royal riders
 **                 are not 'non-passant'. Too hard to do but possibly
 **                 implement as an independent condition later).
 **
 ** 2008/01/01 SE   Bug fix: Circe Assassin + proof game (reported P.Raican)
 **
 ** 2008/02/10 SE   New condition: Cheameleon Pursuit (invented? : L.Grolman)
 **
 ** 2008/02/25 SE   New piece type: Magic
 **                 Adjusted Masand code
 **
 ** 2008/03/13 SE   New condition: Castling Chess (invented: N.A.Bakke?)
 **
 ** 2009/01/03 SE   New condition: Disparate Chess (invented: R.Bedoni)
 **
 **************************** End of List ******************************/

#if defined(macintosh) /* is always defined on macintosh's  SB */
# define SEGM2
# include "platform/unix/mac.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>  /* H.D. 10.02.93 prototype fuer exit */
#include <string.h>

#if defined(DOS)
# if defined(__GNUC__)
#  include <pc.h>
# else
#  include <bios.h>
# endif /* __GNUC__ */
#endif /* DOS */

#include "py.h"
#include "stipulation/stipulation.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "stipulation/has_solution_type.h"
#include "solving/battle_play/attack_play.h"
#include "stipulation/goals/doublemate/reached_tester.h"
#include "stipulation/temporary_hacks.h"
#include "solving/single_piece_move_generator.h"
#include "solving/castling_intermediate_move_generator.h"
#include "conditions/ohneschach/immobility_tester.h"
#include "conditions/exclusive.h"
#include "conditions/extinction.h"
#include "conditions/republican.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "optimisations/hash.h"
#include "debugging/trace.h"
#include "debugging/measure.h"

static piece linechampiece(piece p, square sq)
{
  piece pja= p;
  if (CondFlag[leofamily]) {
    switch (abs(p)) {
    case leob: case maob: case vaob: case paob:
      switch(sq%onerow) {
      case 8:  case 15:   pja= paob; break;
      case 9:  case 14:   pja= maob; break;
      case 10: case 13:   pja= vaob; break;
      case 11:     pja= leob; break;
      }
    }
    return (pja != p && p < vide) ? - pja : pja;
  } else
    if (CondFlag[cavaliermajeur]) {
      switch (abs(p)) {
      case db: case nb: case fb: case tb:
        switch(sq%onerow) {
        case 8:  case 15:   pja= tb;  break;
        case 9:  case 14:   pja= nb;  break;
        case 10: case 13:   pja= fb;  break;
        case 11:     pja= db;  break;
        }
      }
      return (pja != p && p < vide) ? - pja : pja;
    } else {
      switch (abs(p)) {
      case db: case cb: case fb: case tb:
        switch(sq%onerow) {
        case 8:  case 15:   pja= tb;  break;
        case 9:  case 14:   pja= cb;  break;
        case 10: case 13:   pja= fb;  break;
        case 11:     pja= db;  break;
        }
      }
      return (pja != p && p < vide) ? - pja : pja;
    }
} /* linechampiece */


piece champiece(piece p)
{
  /* function realisiert Figurenwechsel bei Chamaeleoncirce */
  if (CondFlag[leofamily])
  {
    switch (p)
    {
      case leob:
        return maob;
      case leon:
        return maon;
      case maob:
        return vaob;
      case maon:
        return vaon;
      case vaob:
        return paob;
      case vaon:
        return paon;
      case paob:
        return leob;
      case paon:
        return leon;
      default:
        break;
    }
  }
  else if (CondFlag[cavaliermajeur])
  {
    switch (p)
    {
      case db:
        return nb;
      case dn:
        return nn;
      case nb:
        return fb;
      case nn:
        return fn;
      case fb:
        return tb;
      case fn:
        return tn;
      case tb:
        return db;
      case tn:
        return dn;
      default:
        break;
    }
  }
  else
  {
    switch (p)
    {
      case db:
        return cb;
      case dn:
        return cn;
      case cb:
        return fb;
      case cn:
        return fn;
      case fb:
        return tb;
      case fn:
        return tn;
      case tb:
        return db;
      case tn:
        return dn;
      default:
        break;
    }
  }

  return p;
}

piece norskpiece(piece p)
{
  /* function realisiert Figurenwechsel bei NorskSjakk */
  if (CondFlag[leofamily]) {
    switch (p)
    {
      case leob:
        return maob;
      case leon:
        return maon;
      case maob:
        return leob;
      case maon:
        return leon;
      case vaob:
        return paob;
      case vaon:
        return paon;
      case paob:
        return vaob;
      case paon:
        return vaon;
      default:
        break;
    }
  }
  else if (CondFlag[cavaliermajeur])
  {
    switch (p)
    {
      case db:
        return nb;
      case dn:
        return nn;
      case nb:
        return db;
      case nn:
        return dn;
      case fb:
        return tb;
      case fn:
        return tn;
      case tb:
        return fb;
      case tn:
        return fn;
      default:
        break;
    }
  }
  else
  {
    switch (p)
    {
      case db:
        return cb;
      case dn:
        return cn;
      case cb:
        return db;
      case cn:
        return dn;
      case fb:
        return tb;
      case fn:
        return tn;
      case tb:
        return fb;
      case tn:
        return fn;
      default:
        break;
    }
  }

  return p;
} /* norskpiece */

piece dec_einstein(piece p)
{
  /* function realisiert Figurenwechsel bei Einsteinschach abwaerts */
  switch (p)
  {
  case db:
    return tb;
    case dn:
      return tn;
    case tb:
      return fb;
    case tn:
      return fn;
    case fb:
      return cb;
    case fn:
      return cn;
    case cb:
      return pb;
    case cn:
      return pn;
    default:
      break;
  }

  return p;
}

piece inc_einstein(piece p)
{
  /* function realisiert Figurenwechsel bei Einsteinschach aufwaerts */
  switch (p) {
    case pb:
      return cb;
    case pn:
      return cn;
    case cb:
      return fb;
    case cn:
      return fn;
    case fb:
      return tb;
    case fn:
      return tn;
    case tb:
      return db;
    case tn:
      return dn;
    default:
      break;
  }

  return p;
}

#if defined(DOS)
# pragma warn -par
#endif

square renplus(ply ply_id,
               piece p_captured, Flags p_captured_spec,
               square sq_capture, square sq_departure, square sq_arrival,
               Side capturer) {
  /* Echecs plus */
  if (sq_capture==square_d4
      || sq_capture==square_e4
      || sq_capture==square_d5
      || sq_capture==square_e5)
  {
    square k;
    switch (mars_circe_rebirth_state)
    {
      case 0:
        k= square_d4;
        break;

      case 1:
        k= square_e4;
        break;

      case 2:
        k= square_d5;
        break;

      case 3:
        k= square_e5;
        break;

      default:
        k= initsquare;
        break;
    }

    mars_circe_rebirth_state = (mars_circe_rebirth_state+1)%4;

    return k;
  }
  else
    return sq_capture;
}

square renrank(ply ply_id,
               piece p_captured, Flags p_captured_spec,
               square sq_capture, square sq_departure, square sq_arrival,
               Side capturer) {
  square sq= ((sq_capture/onerow)%2==1
              ? rennormal(ply_id,p_captured,p_captured_spec,
                          sq_capture,sq_departure,sq_arrival,capturer)
              : renspiegel(ply_id,p_captured,p_captured_spec,
                           sq_capture,sq_departure,sq_arrival,capturer));
  return onerow*(sq_capture/onerow) + sq%onerow;
}

square renfile(ply ply_id,
               piece p_captured, Flags p_captured_spec,
               square sq_capture, square sq_departure, square sq_arrival,
               Side capturer)
{
  int col= sq_capture % onerow;

  if (capturer==Black)
  {
    if (is_pawn(p_captured))
      return col + (nr_of_slack_rows_below_board+1)*onerow;
    else
      return col + nr_of_slack_rows_below_board*onerow;
  }
  else
  {
    if (is_pawn(p_captured))
      return col + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
    else
      return col + (nr_of_slack_rows_below_board+nr_rows_on_board-1)*onerow;
  }
}

square renspiegelfile(ply ply_id,
                      piece p_captured, Flags p_captured_spec,
                      square sq_capture,
                      square sq_departure, square sq_arrival,
                      Side capturer)
{
  return renfile(ply_id,p_captured,p_captured_spec,
                 sq_capture,sq_departure,sq_arrival,advers(capturer));
} /* renspiegelfile */

square renpwc(ply ply_id,
              piece p_captured, Flags p_captured_spec,
              square sq_capture, square sq_departure, square sq_arrival,
              Side capturer)
{
  return sq_departure;
} /* renpwc */

square renequipollents(ply ply_id,
                       piece p_captured, Flags p_captured_spec,
                       square sq_capture,
                       square sq_departure, square sq_arrival,
                       Side capturer)
{
  /* we have to solve the enpassant capture / locust capture problem in the future. */
#if defined(WINCHLOE)
  return sq_capture + sq_arrival - sq_departure;
#endif
  return sq_capture + sq_capture - sq_departure;
} /* renequipollents */

square renequipollents_anti(ply ply_id,
                            piece p_captured, Flags p_captured_spec,
                            square sq_capture,
                            square sq_departure, square sq_arrival,
                            Side capturer)
{
  /* we have to solve the enpassant capture / locust capture problem in the future. */
#if defined(WINCHLOE)
  return sq_arrival + sq_arrival - sq_departure;
#endif
  return sq_capture + sq_capture - sq_departure;
} /* renequipollents_anti */

square rensymmetrie(ply ply_id,
                    piece p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer)
{
  return (square_h8+square_a1) - sq_capture;
} /* rensymmetrie */

square renantipoden(ply ply_id,
                    piece p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer)
{
  int const row= sq_capture/onerow - nr_of_slack_rows_below_board;
  int const file= sq_capture%onerow - nr_of_slack_files_left_of_board;

  sq_departure= sq_capture;

  if (row<nr_rows_on_board/2)
    sq_departure+= nr_rows_on_board/2*dir_up;
  else
    sq_departure+= nr_rows_on_board/2*dir_down;

  if (file<nr_files_on_board/2)
    sq_departure+= nr_files_on_board/2*dir_right;
  else
    sq_departure+= nr_files_on_board/2*dir_left;

  return sq_departure;
} /* renantipoden */

square rendiagramm(ply ply_id,
                   piece p_captured, Flags p_captured_spec,
                   square sq_capture, square sq_departure, square sq_arrival,
                   Side capturer)
{
  return GetPositionInDiagram(p_captured_spec);
}

square rennormal(ply ply_id,
                 piece p_captured, Flags p_captured_spec,
                 square sq_capture, square sq_departure, square sq_arrival,
                 Side capturer)
{
  square  Result;
  int col, ran;
  Side  cou;
  PieNam pnam_captured = abs(p_captured);

  col = sq_capture % onerow;
  ran = sq_capture / onerow;

  if (CondFlag[circemalefiquevertical]) {
    col= onerow-1 - col;
    if (pnam_captured==Queen)
      pnam_captured = King;
    else if (pnam_captured==King)
      pnam_captured = Queen;
  }

  if ((ran&1) != (col&1))
    cou = White;
  else
    cou = Black;

  if (CondFlag[cavaliermajeur])
    if (pnam_captured==NightRider)
      pnam_captured = Knight;

  /* Below is the reason for the define problems. What a "hack" ! */
  if (CondFlag[leofamily]
      && pnam_captured>=Leo && Vao>=pnam_captured)
    pnam_captured -= 4;

  if (capturer == Black)
  {
    if (is_pawn(pnam_captured))
      Result= col + (nr_of_slack_rows_below_board+1)*onerow;
    else {
      if (CondFlag[frischauf] && TSTFLAG(p_captured_spec,FrischAuf)) {
        Result= (col
                 + (onerow
                    *(CondFlag[glasgow]
                      ? nr_of_slack_rows_below_board+nr_rows_on_board-2
                      : nr_of_slack_rows_below_board+nr_rows_on_board-1)));
      }
      else
        switch(pnam_captured) {
        case King:
          Result= square_e1;
          break;
        case Knight:
          Result= cou == White ? square_b1 : square_g1;
          break;
        case Rook:
          Result= cou == White ? square_h1 : square_a1;
          break;
        case Queen:
          Result= square_d1;
          break;
        case Bishop:
          Result= cou == White ? square_f1 : square_c1;
          break;
        default: /* fairy piece */
          Result= (col
                   + (onerow
                      *(CondFlag[glasgow]
                        ? nr_of_slack_rows_below_board+nr_rows_on_board-2
                        : nr_of_slack_rows_below_board+nr_rows_on_board-1)));
        }
    }
  }
  else
  {
    if (is_pawn(pnam_captured))
      Result= col + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
    else {
      if (CondFlag[frischauf] && TSTFLAG(p_captured_spec,FrischAuf)) {
        Result= (col
                 + (onerow
                    *(CondFlag[glasgow]
                      ? nr_of_slack_rows_below_board+1
                      : nr_of_slack_rows_below_board)));
      }
      else
        switch(pnam_captured) {
        case Bishop:
          Result= cou == White ? square_c8 : square_f8;
          break;
        case Queen:
          Result= square_d8;
          break;
        case Rook:
          Result= cou == White ? square_a8 : square_h8;
          break;
        case Knight:
          Result= cou == White ? square_g8 : square_b8;
          break;
        case King:
          Result= square_e8;
          break;
        default: /* fairy piece */
          Result= (col
                   + (onerow
                      *(CondFlag[glasgow]
                        ? nr_of_slack_rows_below_board+1
                        : nr_of_slack_rows_below_board)));
        }
    }
  }

  return(Result);
} /* rennormal */

square rendiametral(ply ply_id,
                    piece p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer) {
  return (square_h8+square_a1
          - rennormal(ply_id,p_captured,p_captured_spec,
                      sq_capture,sq_departure,sq_arrival,capturer));
}

square renspiegel(ply ply_id,
                  piece p_captured, Flags p_captured_spec,
                  square sq_capture,
                  square sq_departure, square sq_arrival,
                  Side capturer)
{
  return rennormal(ply_id,p_captured,p_captured_spec,
                   sq_capture,sq_departure,sq_arrival,advers(capturer));
}

square rensuper(ply ply_id,
                piece p_captured, Flags p_captured_spec,
                square sq_capture,
                square sq_departure, square sq_arrival,
                Side capturer)
{
  return super[ply_id];
}

#if defined(DOS)
# pragma warn +par
#endif

boolean is_short(piece p)
{
  switch (abs(p)) {
  case  Pawn:
  case  BerolinaPawn:
  case  ReversePawn:
  case  Mao:
  case  Moa:
  case  Skylla:
  case  Charybdis:
  case  ChinesePawn:
    return  true;
  default:
    return  false;
  }
}

boolean is_pawn(piece p)
{
  switch (abs(p)) {
  case  Pawn:
  case  BerolinaPawn:
  case  SuperBerolinaPawn:
  case  SuperPawn:
  case  ReversePawn:
    return  true;
  default:
    return  false;
  }
}

boolean is_forwardpawn(piece p)
{
  switch (abs(p)) {
  case  Pawn:
  case  BerolinaPawn:
  case  SuperBerolinaPawn:
  case  SuperPawn:
    return  true;
  default:
    return  false;
  }
}

boolean is_reversepawn(piece p)
{
  switch (abs(p)) {
  case  ReversePawn:
    return  true;
  default:
    return  false;
  }
}

/* Determine whether a sequence of squares are empty
 * @param from start of sequence
 * @param to end of sequence
 * @param direction delta to (repeatedly) apply to reach to from from
 * @return true if the squares between (and not including) from and to are empty
 */
static boolean are_squares_empty(square from, square to, int direction)
{
  square s;
  for (s = from+direction; s!=to; s += direction)
    if (e[s]!=vide)
      return false;

  return true;
}

boolean castling_is_intermediate_king_move_legal(Side side, square from, square to)
{
  boolean result = false;

  if (complex_castling_through_flag)
  {
    /* temporarily deactivate maximummer etc. */
    boolean const save_flagmummer = flagmummer[side];
    flagmummer[side] = false;
    castling_intermediate_move_generator_init_next(from,to);
    result = attack(slices[temporary_hack_castling_intermediate_move_legality_tester[side]].next2,length_unspecified)==has_solution;
    flagmummer[side] = save_flagmummer;
  }
  else
  {
    piece const sides_king = side==White ? roib : roin;
    e[from]= vide;
    e[to]= sides_king;
    if (king_square[side]!=initsquare)
      king_square[side] = to;

    result = !echecc(nbply,side);

    e[from]= sides_king;
    e[to]= vide;
    if (king_square[side]!=initsquare)
      king_square[side] = from;
  }

  return result;
}

void generate_castling(Side side)
{
  /* It works only for castling_supported == TRUE
     have a look at funtion verifieposition() in py6.c
  */

  square const square_a = side==White ? square_a1 : square_a8;
  square const square_e = square_a+file_e;
  piece const sides_king = side==White ? roib : roin;

  if (dont_generate_castling)
    return;

  if (TSTCASTLINGFLAGMASK(nbply,side,castlings)>k_cancastle
      && e[square_e]==sides_king
      /* then the king on e8 and at least one rook can castle !! */
      && !echecc(nbply,side))
  {
    square const square_c = square_a+file_c;
    square const square_d = square_a+file_d;
    square const square_f = square_a+file_f;
    square const square_g = square_a+file_g;
    square const square_h = square_a+file_h;
    piece const sides_rook = side==White ? tb : tn;

    /* 0-0 */
    if (TSTCASTLINGFLAGMASK(nbply,side,k_castling)==k_castling
        && e[square_h]==sides_rook
        && are_squares_empty(square_e,square_h,dir_right)
        && castling_is_intermediate_king_move_legal(side,square_e,square_f))
      empile(square_e,square_g,kingside_castling);

    /* 0-0-0 */
    if (TSTCASTLINGFLAGMASK(nbply,side,q_castling)==q_castling
        && e[square_a]==sides_rook
        && are_squares_empty(square_e,square_a,dir_left)
        && castling_is_intermediate_king_move_legal(side,square_e,square_d))
      empile(square_e,square_c,queenside_castling);
  }
}

void genrn(square sq_departure)
{
  Side const side = Black;
  boolean flag = false;  /* K im Schach ? */
  numecoup const save_nbcou = nbcou;

  if (calc_refl_king[side] && !calctransmute)
  {
    /* K im Schach zieht auch */
    calctransmute = true;
    if (!normaltranspieces[side] && echecc(nbply,side))
    {
      piece *ptrans;
      for (ptrans = transmpieces[side]; *ptrans!=vide; ++ptrans)
      {
        flag = true;
        current_trans_gen = -*ptrans;
        gen_bl_piece(sq_departure,-*ptrans);
        current_trans_gen = vide;
      }
    }
    else if (normaltranspieces[side])
    {
      piece const *ptrans;
      for (ptrans = transmpieces[side]; *ptrans!=vide; ++ptrans)
        if (nbpiece[*ptrans]>0
            && (*checkfunctions[*ptrans])(sq_departure,*ptrans,eval_black))
        {
          flag = true;
          current_trans_gen = -*ptrans;
          gen_bl_piece(sq_departure,-*ptrans);
          current_trans_gen = vide;
        }
    }
    calctransmute = false;

    if (flag && nbpiece[orphanb]>0)
    {
      piece const king = e[king_square[side]];
      e[king_square[side]] = dummyn;
      if (!echecc(nbply,side))
        /* side's king checked only by an orphan empowered by the king */
        flag = false;
      e[king_square[side]] = king;
    }

    /* K im Schach zieht nur */
    if (calc_trans_king[side] && flag)
      return;
  }

  if (CondFlag[sting])
    gerhop(sq_departure,vec_queen_start,vec_queen_end,side);

  {
    numvec k;
    for (k = vec_queen_end; k>=vec_queen_start; --k)
    {
      square const sq_arrival = sq_departure+vec[k];
      if (e[sq_arrival]==vide || e[sq_arrival]>=roib)
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }

  if (flag)
    remove_duplicate_moves(save_nbcou);

  /* Now we test castling */
  if (castling_supported)
    generate_castling(side);

  if (CondFlag[castlingchess] && !echecc(nbply,side))
  {
    numvec k;
    for (k = vec_queen_end; k>= vec_queen_start; --k)
    {
      square const sq_passed = sq_departure+vec[k];
      square const sq_arrival = sq_passed+vec[k];
      square sq_castler;
      piece p;

      finligne(sq_departure,vec[k],p,sq_castler);
      if (sq_castler!=sq_passed && sq_castler!=sq_arrival && abs(p)>=roib
          && castling_is_intermediate_king_move_legal(side,sq_departure,sq_passed))
        empile(sq_departure,sq_arrival,maxsquare+sq_castler);
    }
  }

  if (CondFlag[platzwechselrochade] && blpwr[nbply])
  {
    int i,j;
    piece p;
    square z= square_a1;
    for (i= nr_rows_on_board; i > 0; i--, z+= onerow-nr_files_on_board)
    for (j= nr_files_on_board; j > 0; j--, z++) {
      if ((p = e[z]) != vide) {
      if (TSTFLAG(spec[z], Neutral))
        p = -p;
      if (p < vide && !is_pawn(p))  /* not sure if "castling" with Ps forbidden */
        empile(sq_departure,z,platzwechsel_rochade);
      }
    }
  }
}

void gen_bl_ply(void)
{
  square i, j, z;
  piece p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* Don't try to "optimize" by hand. The double-loop is tested as the  */
  /* fastest way to compute (due to compiler-optimizations !) */
  z= square_h8;
  for (i= nr_rows_on_board; i > 0; i--, z-= onerow-nr_files_on_board)
    for (j= nr_files_on_board; j > 0; j--, z--) {
      if ((p = e[z]) != vide) {
        if (TSTFLAG(spec[z], Neutral))
          p = -p;
        if (p < vide)
          gen_bl_piece(z, p);
      }
    }
  if (blacknull)
    empile(nullsquare, nullsquare, nullsquare);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* gen_bl_ply */

void gen_bl_piece_aux(square z, piece p)
{
  if (CondFlag[annan]) {
    piece annan_p= e[z+onerow];
    if (blannan(z+onerow, z))
      p= annan_p;
  }

  switch(p) {
  case roin: genrn(z);
    break;
  case pn: genpn(z);
    break;
  case cn: genleap(z, vec_knight_start,vec_knight_end);
    break;
  case tn: genrid(z, vec_rook_start,vec_rook_end);
    break;
  case dn: genrid(z, vec_queen_start,vec_queen_end);
    break;
  case fn: genrid(z, vec_bishop_start,vec_bishop_end);
    break;
  default: gfeernoir(z, p);
    break;
  }
}

static void orig_gen_bl_piece(square sq_departure, piece p)
{
  piece pi_departing;

  if (flag_libre_on_generate && !libre(sq_departure,true))
    return;

  if (TSTFLAG(PieSpExFlags,Paralyse)) {
    if (paralysiert(sq_departure)) {
      return;
    }
  }

  if (anymars||anyantimars) {
    square sq_rebirth;
    Flags spec_departing;

    if (CondFlag[phantom]) {
      numecoup const anf1 = nbcou;
      /* generate standard moves first */
      flagactive= false;
      flagpassive= false;
      flagcapture= false;

      gen_bl_piece_aux(sq_departure,p);

      /* Kings normally don't move from their rebirth-square */
      if (p == e[king_square[Black]] && !rex_phan) {
        return;
      }
      /* generate moves from rebirth square */
      flagactive= true;
      spec_departing=spec[sq_departure];
      sq_rebirth= (*marsrenai)(nbply,
                               p,
                               spec_departing,
                               sq_departure,
                               initsquare,
                               initsquare,
                               White);
      /* if rebirth square is where the piece stands,
         we've already generated all the relevant moves.
      */
      if (sq_rebirth==sq_departure)
        return;
      if (e[sq_rebirth] == vide)
      {
        numecoup const anf2 = nbcou;
        numecoup l1;
        pi_departing=e[sq_departure];   /* Mars/Neutral bug */
        e[sq_departure]= vide;
        spec[sq_departure]= EmptySpec;
        spec[sq_rebirth]= spec_departing;
        e[sq_rebirth]= p;
        marsid= sq_departure;

        gen_bl_piece_aux(sq_rebirth, p);

        e[sq_rebirth]= vide;
        spec[sq_departure]= spec_departing;
        e[sq_departure]= pi_departing;
        flagactive= false;
        /* Unfortunately we have to check for
           duplicate generated moves now.
           there's only ONE duplicate per arrival square
           possible !
        */
        for (l1 = anf1+1; l1<=anf2; l1++)
        {
          numecoup l2 = anf2+1;
          while (l2 <= nbcou)
            if (move_generation_stack[l1].arrival
                ==move_generation_stack[l2].arrival)
            {
              move_generation_stack[l2] = move_generation_stack[nbcou];
              --nbcou;
              break;  /* remember: ONE duplicate ! */
            }
            else
              l2++;
        }
      }
    }
    else {
      /* generate noncapturing moves first */
      flagpassive= true;
      flagcapture= false;

      gen_bl_piece_aux(sq_departure, p);

      /* generate capturing moves now */
      flagpassive= false;
      flagcapture= true;
      mars_circe_rebirth_state = 0;
      do {   /* Echecs Plus */
        spec_departing= spec[sq_departure];
        sq_rebirth= (*marsrenai)(nbply,
                                 p,
                                 spec_departing,
                                 sq_departure,
                                 initsquare,
                                 initsquare,
                                 White);
        if (sq_rebirth==sq_departure || e[sq_rebirth]==vide) {
          pi_departing= e[sq_departure]; /* Mars/Neutral bug */

          e[sq_departure]= vide;
          spec[sq_departure]= EmptySpec;

          spec[sq_rebirth]= spec_departing;
          e[sq_rebirth]= p;

          marsid= sq_departure;

          gen_bl_piece_aux(sq_rebirth,p);

          e[sq_rebirth]= vide;

          spec[sq_departure]= spec_departing;
          e[sq_departure]= pi_departing;
        }
      } while (mars_circe_rebirth_state);
      flagcapture= false;
    }
  }
  else
    gen_bl_piece_aux(sq_departure,p);

  if (CondFlag[messigny] && !(king_square[Black]==sq_departure && rex_mess_ex))
  {
    square const *bnp;
    for (bnp= boardnum; *bnp; bnp++)
      if (e[*bnp]==-p)
        empile(sq_departure,*bnp,messigny_exchange);
  }
} /* orig_gen_bl_piece */

void singleboxtype3_gen_bl_piece(square z, piece p) {
  numecoup save_nbcou = nbcou;
  unsigned int latent_prom = 0;
  square sq;
  for (sq = next_latent_pawn(initsquare,Black);
       sq!=initsquare;
       sq = next_latent_pawn(sq,Black))
  {
    piece pprom;
    for (pprom = next_singlebox_prom(vide,Black);
         pprom!=vide;
         pprom = next_singlebox_prom(pprom,Black))
    {
      numecoup prev_nbcou = nbcou;
      ++latent_prom;
      e[sq] = -pprom;
      orig_gen_bl_piece(z, sq==z ? -pprom : p);
      e[sq] = pn;

      for (++prev_nbcou; prev_nbcou<=nbcou; ++prev_nbcou)
      {
        sb3[prev_nbcou].where = sq;
        sb3[prev_nbcou].what = -pprom;
      }
    }
  }

  if (latent_prom==0)
  {
    orig_gen_bl_piece(z,p);
    for (++save_nbcou; save_nbcou<=nbcou; ++save_nbcou)
    {
      sb3[save_nbcou].where = initsquare;
      sb3[save_nbcou].what = vide;
    }
  }
}

void (*gen_bl_piece)(square z, piece p) = &orig_gen_bl_piece;

void genmove(Side camp)
{
  /* TODO hide away in one module per platform */
  /* Abbruch waehrend der gesammten Laufzeit mit <ESC> */
#if defined(ATARI)
# include <osbind.h>
# define STOP_ON_ESC
# define interupt (Bconstat(2) && (Bconin(2) == 27))
#endif /* ATARI */

#if defined(DOS)
#  if !defined(Windows)
#    if defined(__TURBOC__)
#      define STOP_ON_ESC
#      define interupt ((bioskey(1) != 0) && ((bioskey(0) >> 8) == 1))
#    endif /* __TURBOC__ */
#  endif

#  if defined(_MSC_VER)
#    define STOP_ON_ESC
#    define interupt (_bios_keybrd(_KEYBRD_READY) && ((_bios_keybrd(_KEYBRD_READ) >> 8) == 1))
#  endif /* _MSC_VER */

#  if defined(__GNUC__)
#    define STOP_ON_ESC
#    define interupt (kbhit() && (getkey() == 27)) /* ESC == 27 */
#  endif /* __GNUC__ */
#endif /* DOS */

#if defined(STOP_ON_ESC)
  if (interupt) {
    StdString(GetMsgString(InterMessage));
    StdString(" ");
    PrintTime();
    StdString("\n\n");
    CloseInput();
    /* for some other purposes I need a return value
    ** different from 0.  TLi
    ** exit(0);
    */
    exit(1);
  }
#endif /* STOP_ON_ESC */

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,camp,"");
  TraceFunctionParamListEnd();

  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(advers(camp));
  if (nbply==1 && flag_magic)
    PushMagicViews();
  nextply(nbply);
  trait[nbply]= camp;
  we_generate_exact = false;
  init_move_generation_optimizer();

  if (CondFlag[exclusive])
    exclusive_init_genmove(camp);

  /* exact and consequent maximummers */
  if (camp == White)
  {
    /* let's first generate consequent moves */
    if (wh_exact)
    {
      we_generate_exact = true;
      there_are_consmoves = false;
      gen_wh_ply();
      if (CondFlag[whforsqu] && CondFlag[whconforsqu]
          && !there_are_consmoves) {
        /* There are no consequent moves.
        ** Now let's look for ``normal'' forced moves,
        ** but first reset nbcou etc.
        */
        we_generate_exact = false;
        finply();
        nextply(nbply);
        init_move_generation_optimizer();
        gen_wh_ply();
      }
      we_generate_exact = false;
    }
    else
      gen_wh_ply();
  }
  else
  {
    /* let's first generate consequent moves */
    if (bl_exact)
    {
      we_generate_exact = true;
      there_are_consmoves = false;
      gen_bl_ply();
      if (CondFlag[blforsqu] && CondFlag[blconforsqu]
          && !there_are_consmoves) {
        /* There are no consequent moves.
        ** Now let's look for ``normal'' forced moves,
        ** but first reset nbcou etc.
        */
        we_generate_exact = false;
        finply();
        nextply(nbply);
        init_move_generation_optimizer();
        gen_bl_ply();
      }
      we_generate_exact = false;
    }
    else
      gen_bl_ply();
  }
  finish_move_generation_optimizer();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* genmove(camp) */

static void joueparrain(ply ply_id)
{
  numecoup const coup_id = ply_id==nbply ? nbcou : repere[ply_id+1];
  piece const pi_captured = pprise[ply_id-1];
  Flags spec_captured = pprispec[ply_id-1];
  square sq_rebirth;
  if (CondFlag[parrain]) {
	sq_rebirth = (move_generation_stack[repere[ply_id]].capture
                               + move_generation_stack[coup_id].arrival
                               - move_generation_stack[coup_id].departure);
  }
  if (CondFlag[contraparrain]) {
	sq_rebirth = (move_generation_stack[repere[ply_id]].capture
                               - move_generation_stack[coup_id].arrival
                               + move_generation_stack[coup_id].departure);
  }

  if (e[sq_rebirth]==vide)
  {
    sqrenais[ply_id] = sq_rebirth;
    ren_parrain[ply_id] = pi_captured;
    e[sq_rebirth] = pi_captured;
    spec[sq_rebirth] = spec_captured;

    if ((is_forwardpawn(pi_captured)
         && PromSq(advers(trait[ply_id-1]), sq_rebirth))
        || (is_reversepawn(pi_captured)
            && ReversePromSq(advers(trait[ply_id-1]), sq_rebirth)))
    {
      /* captured white pawn on eighth rank: promotion ! */
      /* captured black pawn on first rank: promotion ! */
      piece pprom = cir_prom[ply_id];

      if (TSTFLAG(spec_captured,Chameleon))
        cir_cham_prom[ply_id] = true;

      if (pprom==vide)
      {
        cir_prom[ply_id] = getprompiece[vide];
        pprom = getprompiece[vide];
      }
      if (pi_captured<vide)
        pprom = -pprom;

      e[sq_rebirth]= pprom;
      nbpiece[pprom]++;
      if (cir_cham_prom[ply_id])
        SETFLAG(spec_captured,Chameleon);
      spec[sq_rebirth]= spec_captured;
    }
    else
      nbpiece[pi_captured]++;

    if (TSTFLAG(spec_captured,Neutral))
      setneutre(sq_rebirth);
  }
}

piece pdisp[maxply+1];
Flags pdispspec[maxply+1];
square sqdep[maxply+1];

static boolean patience_legal()
{
  square bl_last_vacated= initsquare, wh_last_vacated= initsquare;
  ply nply;
  /* n.b. inventor rules that R squares are forbidden after
     castling but not yet implemented */

  for (nply= nbply - 1 ; nply > 1 && !bl_last_vacated ; nply--)
    if (trait[nply] == Black)
      bl_last_vacated= sqdep[nply];
  for (nply= nbply - 1 ; nply > 1 && !wh_last_vacated ; nply--)
    if (trait[nply] == White)
      wh_last_vacated= sqdep[nply];
  return !((wh_last_vacated && e[wh_last_vacated]) ||
           (bl_last_vacated && e[bl_last_vacated]));
}

static int direction(square from, square to)
{
  int dir= to-from;
  int hori= to%onerow-from%onerow;
  int vert= to/onerow-from/onerow;
  int i=7;
  while ((hori%i) || (vert%i))
    i--;

  return dir/i;
}

static square blpc;

static boolean eval_spec(square sq_departure,
                         square sq_arrival,
                         square sq_capture)
{
  return sq_departure==blpc;
}

static boolean att_once(square sq_departure, Side trait_ply, ply ply_id)
{
  int i,j, count=0;
  square square_a = square_a1;

  if (trait_ply == White) {

    square const rb_=king_square[White];
    king_square[White] = sq_departure;

    for (i = nr_rows_on_board; i>0; --i, square_a += dir_up)
    {
      square z = square_a;
      for (j= nr_files_on_board; j>0; --j, z += dir_right)
        if (e[z]<-obs)
        {
          blpc = z;
          if (rbechec(ply_id,eval_spec))
          {
            ++count;
            if (count > 1)
              return false;
          }
        }
      }

      king_square[White] = rb_;

  } else {

    square const rn_=king_square[Black];
    king_square[Black] = sq_departure;

    for (i = nr_rows_on_board; i>0; --i, square_a += dir_up)
    {
      square z = square_a;
      for (j= nr_files_on_board; j>0; --j, z += dir_right)
        if (e[z]<-obs)
        {
          blpc = z;
          if (rnechec(ply_id,eval_spec))
          {
            ++count;
            if (count > 1)
              return false;
          }
        }
      }

      king_square[Black] = rn_;
  }
  return count==1;
}

square next_latent_pawn(square s, Side c) {
  piece pawn;
  int  i, delta;

  pawn=  c==White ? pb : pn;
  delta= c==White ?+dir_left :+dir_right;

  if (s==initsquare) {
    i = 0;
    s = c==White ? square_h8 : square_a1;
  }
  else {
    i = c==White ? square_h8-s+1 : s-square_a1+1;
    s += delta;
  }

  for (; i<8; ++i, s+=delta) {
    if (e[s]==pawn) {
      return s;
    }
  }

  return initsquare;
}

piece next_singlebox_prom(piece p, Side c)
{
  piece pprom;
  piece result = vide;

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceFunctionParam("%u",c);
  TraceFunctionParamListEnd();

  for (pprom = getprompiece[p];
       pprom!=vide;
       pprom = getprompiece[pprom])
    if (pprom!=pb
        && nbpiece[c==White ? pprom : -pprom] < nr_piece(game_array)[pprom])
    {
      result = pprom;
      break;
    }

  TraceFunctionExit(__func__);
  TracePiece(p);
  TraceFunctionResultEnd();
  return result;
}

#if defined(DEBUG)
static  int nbrtimes = 0;
#endif

boolean jouecoup_ortho_test(ply ply_id)
{
  boolean flag;
  boolean jtg1= jouetestgenre1;
  jouetestgenre1= false;
  flag= jouecoup(ply_id,replay);
  jouetestgenre1= jtg1;
  return flag;
}

static boolean jouecoup_legality_test(void)
{
  boolean result = true;

  if (!jouetestgenre)
    result = true;
  else if (jouetestgenre1
           && ((CondFlag[blackultraschachzwang]
                && trait[nbply]==Black
                && !echecc(nbply,White))
               || (CondFlag[whiteultraschachzwang]
                   && trait[nbply]==White
                   && !echecc(nbply,Black))
           ))
    result = false;
  else if (CondFlag[isardam] && !isardam_pos_legal())
    result = false;
  else if (CondFlag[circeassassin] && (sqrenais[nbply]==king_square[White] || sqrenais[nbply]==king_square[Black]))
    result = false;
  else if (are_we_testing_immobility_with_opposite_king_en_prise && king_square[advers(trait[nbply])]==initsquare)
    result = false;
  else if (CondFlag[patience] && !PatienceB && !patience_legal()) /* don't call patience_legal if TypeB as obs > vide ! */
    result = false;

  return result;
}

static ghost_index_type find_ghost(square sq_arrival)
{
  ghost_index_type current = nr_ghosts;
  ghost_index_type result = ghost_not_found;
  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  while (current>0)
  {
    --current;
    if (ghosts[current].ghost_square==sq_arrival)
    {
      result = current;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

static void remember_ghost(square sq_arrival)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  assert(nr_ghosts<ghost_capacity);
  ghosts[nr_ghosts].ghost_square = sq_arrival;
  ghosts[nr_ghosts].ghost_piece = e[sq_arrival];
  ghosts[nr_ghosts].ghost_flags = spec[sq_arrival];
  ghosts[nr_ghosts].hidden = false;
  ++nr_ghosts;
  TraceValue("->%u\n",nr_ghosts);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void preempt_ghost(square sq_arrival)
{
  ghost_index_type const ghost_pos = find_ghost(sq_arrival);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",ghost_pos);
  if (ghost_pos!=ghost_not_found)
    ghosts[ghost_pos].hidden = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unpreempt_ghost(square sq_arrival)
{
  ghost_index_type const ghost_pos = find_ghost(sq_arrival);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",ghost_pos);
  if (ghost_pos!=ghost_not_found)
    ghosts[ghost_pos].hidden = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forget_ghost_at_pos(ghost_index_type ghost_pos)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ghost_pos);
  TraceFunctionParamListEnd();

  assert(ghost_pos!=ghost_not_found);
  assert(nr_ghosts>0);
  --nr_ghosts;

  TraceValue("->%u ",nr_ghosts);
  TraceSquare(ghosts[ghost_pos].ghost_square);
  TracePiece(ghosts[ghost_pos].ghost_piece);
  TraceText("\n");
  memmove(ghosts+ghost_pos, ghosts+ghost_pos+1,
          (nr_ghosts-ghost_pos) * sizeof ghosts[0]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forget_ghost(square sq_arrival)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  forget_ghost_at_pos(find_ghost(sq_arrival));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void summon_ghost(square sq_departure)
{
  ghost_index_type const ghost_pos = find_ghost(sq_departure);
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",ghost_pos);

  if (ghost_pos!=ghost_not_found && !ghosts[ghost_pos].hidden)
  {
    piece const piece_summoned = ghosts[ghost_pos].ghost_piece;
    Flags spec_summoned = ghosts[ghost_pos].ghost_flags;

    if (CondFlag[ghostchess])
      SETFLAG(spec_summoned,Uncapturable);

    e[sq_departure] = piece_summoned;
    spec[sq_departure] = spec_summoned;
    ++nbpiece[piece_summoned];

    if (TSTFLAG(spec_summoned,Neutral))
      setneutre(sq_departure);

    forget_ghost_at_pos(ghost_pos);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void ban_ghost(square sq_departure)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  CLRFLAG(spec[sq_departure],Uncapturable);
  remember_ghost(sq_departure);
  --nbpiece[e[sq_departure]];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean find_non_capturing_move(ply ply_id,
                                       square sq_departure,
                                       Side moving_side,
                                       piece p_moving)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceEnumerator(Side,moving_side,"");
  TracePiece(p_moving);
  TraceFunctionParamListEnd();

  init_single_piece_move_generator(sq_departure,p_moving);
  result = attack(slices[temporary_hack_cagecirce_noncapture_finder[moving_side]].next2,length_unspecified)==has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void circecage_advance_cage_prom_impl(ply ply_id,
                                             square cage,
                                             piece *circecage_next_cage_prom)
{
  Side const moving_side = trait[ply_id];
  Side const prisoner_side = advers(moving_side);
  piece const save_prom = e[cage];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TraceSquare(cage);
  TracePiece(*circecage_next_cage_prom);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,prisoner_side,"\n");

  while (true)
  {
    *circecage_next_cage_prom = getprompiece[*circecage_next_cage_prom];
    if (*circecage_next_cage_prom==vide)
      break;
    else
    {
      TracePiece(*circecage_next_cage_prom);TraceText("\n");
      e[cage] = (prisoner_side==White
                 ? *circecage_next_cage_prom
                 : -*circecage_next_cage_prom);
      if (!find_non_capturing_move(ply_id,cage,prisoner_side,e[cage]))
        break;
    }
  }

  TracePiece(*circecage_next_cage_prom);TraceText("\n");

  e[cage] = save_prom;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void circecage_advance_cage_impl(ply ply_id,
                                        piece pi_captured,
                                        square *nextcage,
                                        piece *circecage_next_cage_prom)
{
  Side const moving_side = trait[ply_id];
  Side const prisoner_side = advers(moving_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TracePiece(pi_captured);
  TraceSquare(*nextcage);
  TracePiece(*circecage_next_cage_prom);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,prisoner_side,"\n");

  while (true)
  {
    ++*nextcage;
    if (*nextcage>square_h8)
      break;
    else
    {
      TraceSquare(*nextcage);TraceText("\n");
      if (e[*nextcage]==vide)
      {
        if (is_pawn(pi_captured) && PromSq(prisoner_side,*nextcage))
        {
          circecage_advance_cage_prom_impl(ply_id,*nextcage,circecage_next_cage_prom);
          if (*circecage_next_cage_prom!=vide)
            break;
        }
        else
        {
          boolean cage_found;

          e[*nextcage] = pi_captured;

          cage_found = !find_non_capturing_move(ply_id,
                                                *nextcage,
                                                prisoner_side,
                                                pi_captured);

          e[*nextcage] = vide;

          if (cage_found)
            break;
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void circecage_advance_norm_prom_impl(ply ply_id,
                                             square sq_arrival, piece pi_captured,
                                             square *nextcage,
                                             piece *circecage_next_cage_prom,
                                             piece *circecage_next_norm_prom)
{
  Side const moving_side = trait[ply_id];
  piece const save_prom = e[sq_arrival];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TraceSquare(sq_arrival);
  TracePiece(pi_captured);
  TraceSquare(*nextcage);
  TracePiece(*circecage_next_cage_prom);
  TracePiece(*circecage_next_norm_prom);
  TraceFunctionParamListEnd();

  *circecage_next_norm_prom = getprompiece[*circecage_next_norm_prom];
  TracePiece(*circecage_next_norm_prom);TraceText("\n");

  if (*circecage_next_norm_prom!=vide)
  {
    e[sq_arrival] = (moving_side==White
                     ? *circecage_next_norm_prom
                     : -*circecage_next_norm_prom);
    ++nbpiece[e[sq_arrival]];
    circecage_advance_cage_impl(nbply,
                                pi_captured,
                                nextcage,
                                circecage_next_cage_prom);
    --nbpiece[e[sq_arrival]];
  }

  e[sq_arrival] = save_prom;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void circecage_find_initial_cage_impl(ply ply_id,
                                             piece pi_departing,
                                             square sq_arrival, piece pi_captured,
                                             square *nextcage,
                                             piece *circecage_next_cage_prom,
                                             piece *circecage_next_norm_prom)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TracePiece(pi_departing);
  TraceSquare(sq_arrival);
  TracePiece(pi_captured);
  TraceSquare(*nextcage);
  TracePiece(*circecage_next_cage_prom);
  TracePiece(*circecage_next_norm_prom);
  TraceFunctionParamListEnd();

  if (is_pawn(pi_departing) && PromSq(trait[ply_id],sq_arrival))
    circecage_advance_norm_prom_impl(ply_id,
                                     sq_arrival,pi_captured,
                                     nextcage,
                                     circecage_next_cage_prom,
                                     circecage_next_norm_prom);
  else
    circecage_advance_cage_impl(ply_id,
                                pi_captured,
                                nextcage,
                                circecage_next_cage_prom);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean circecage_are_we_finding_cage = false;

static void circecage_advance_cage(ply ply_id,
                                   piece pi_captured,
                                   square *nextcage,
                                   piece *circecage_next_cage_prom)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TracePiece(pi_captured);
  TraceSquare(*nextcage);
  TracePiece(*circecage_next_cage_prom);
  TraceFunctionParamListEnd();

  if (circecage_are_we_finding_cage)
    *nextcage = square_h8+1;
  else
  {
    circecage_are_we_finding_cage = true;
    circecage_advance_cage_impl(ply_id,pi_captured,nextcage,circecage_next_cage_prom);
    circecage_are_we_finding_cage = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void circecage_advance_norm_prom(ply ply_id,
                                        square sq_arrival, piece pi_captured,
                                        square *nextcage,
                                        piece *circecage_next_cage_prom,
                                        piece *circecage_next_norm_prom)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TraceSquare(sq_arrival);
  TracePiece(pi_captured);
  TraceSquare(*nextcage);
  TracePiece(*circecage_next_cage_prom);
  TracePiece(*circecage_next_norm_prom);
  TraceFunctionParamListEnd();

  if (circecage_are_we_finding_cage)
    *circecage_next_norm_prom = vide;
  else
  {
    circecage_are_we_finding_cage = true;
    circecage_advance_norm_prom_impl(ply_id,sq_arrival,pi_captured,nextcage,circecage_next_cage_prom,circecage_next_norm_prom);
    circecage_are_we_finding_cage = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void circecage_advance_cage_prom(ply ply_id,
                                        square cage,
                                        piece *circecage_next_cage_prom)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TraceSquare(cage);
  TracePiece(*circecage_next_cage_prom);
  TraceFunctionParamListEnd();

  if (circecage_are_we_finding_cage)
    *circecage_next_cage_prom = vide;
  else
  {
    circecage_are_we_finding_cage = true;
    circecage_advance_cage_prom_impl(ply_id,cage,circecage_next_cage_prom);
    circecage_are_we_finding_cage = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void circecage_find_initial_cage(ply ply_id,
                                        piece pi_departing,
                                        square sq_arrival, piece pi_captured,
                                        square *nextcage,
                                        piece *circecage_next_cage_prom,
                                        piece *circecage_next_norm_prom)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TracePiece(pi_departing);
  TraceSquare(sq_arrival);
  TracePiece(pi_captured);
  TraceSquare(*nextcage);
  TracePiece(*circecage_next_cage_prom);
  TracePiece(*circecage_next_norm_prom);
  TraceFunctionParamListEnd();


  *nextcage = superbas;

  if (!circecage_are_we_finding_cage)
  {
    circecage_are_we_finding_cage = true;
    circecage_find_initial_cage_impl(ply_id,
                                     pi_departing,sq_arrival,pi_captured,
                                     nextcage,
                                     circecage_next_cage_prom,
                                     circecage_next_norm_prom);
    circecage_are_we_finding_cage = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

square rencage(ply ply_id,
               piece p_captured, Flags p_captured_spec,
               square sq_capture,
               square sq_departure, square sq_arrival,
               Side capturer)
{
  square result = superbas;
  piece nextcageprom = vide;
  piece nextnormprom = vide;
  piece const pi_departing = e[sq_departure];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TracePiece(p_captured);
  TraceSquare(sq_capture);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  TracePiece(e[sq_departure]);
  TracePiece(e[sq_capture]);
  TracePiece(e[sq_arrival]);
  TraceText("\n");

  if (!circecage_are_we_finding_cage)
  {
    circecage_are_we_finding_cage = true;

    e[sq_departure] = vide;
    e[sq_capture] = vide;
    e[sq_arrival] = pi_departing;

    circecage_find_initial_cage_impl(ply_id,
                                     pi_departing,sq_capture,p_captured,
                                     &result,&nextcageprom,&nextnormprom);

    e[sq_arrival] = vide;
    e[sq_capture] = p_captured;
    e[sq_departure] = pi_departing;

    circecage_are_we_finding_cage = false;
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

DEFINE_COUNTER(jouecoup)

boolean jouecoup(ply ply_id, joue_type jt)
{
  square sq_rebirth = initsquare;
  piece pi_reborn;

  square sq_hurdle = initsquare;
  piece pi_hurdle;

  piece pi_captured;
  Flags spec_pi_captured;

  boolean rochade = false;
  boolean platzwechsel = false;

  Side const trait_ply = trait[ply_id];

#if defined(DEBUG)
  nbrtimes++;
#endif

  numecoup const coup_id = ply_id==nbply ? nbcou : repere[ply_id+1];
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;

  square const prev_rb = king_square[White];
  square const prev_rn = king_square[Black];

  square const sq_arrival = move_gen_top->arrival;
  square sq_capture = move_gen_top->capture;

  square const sq_departure = move_gen_top->departure;

  Flags spec_pi_moving = spec[sq_departure];
  piece pi_arriving = e[sq_departure];

  piece pi_departing = pi_arriving;

  INCREMENT_COUNTER(jouecoup);

  RB_[ply_id] = king_square[White];
  RN_[ply_id] = king_square[Black];

  pjoue[ply_id] = pi_arriving;
  jouespec[ply_id] = spec_pi_moving;
  sqdep[ply_id] = sq_departure;

  if (jt!=replay)
    invalidateHashBuffer();

  if (jouegenre)
  {
    rochade_sq[coup_id] = initsquare;
    if (sq_capture > platzwechsel_rochade)
    {
      rochade_sq[coup_id] = sq_capture-maxsquare;
      rochade_pc[coup_id] = e[rochade_sq[coup_id]];
      rochade_sp[coup_id] = spec[rochade_sq[coup_id]];
      sq_capture = sq_arrival;
      rochade = true;
    }
    else if (sq_capture == platzwechsel_rochade)
    {
      rochade_sq[coup_id] = sq_arrival;
      rochade_pc[coup_id] = e[rochade_sq[coup_id]];
      rochade_sp[coup_id] = spec[rochade_sq[coup_id]];
      rochade_sq[coup_id] = -sq_arrival; /* hack for output */
      sq_capture = sq_arrival;
      platzwechsel = true;
      if (trait_ply == White)
        whpwr[nbply]= false;
      else
        blpwr[nbply]=false;
    }

    if (CondFlag[amu])
      att_1[ply_id]= att_once(sq_departure, trait_ply, ply_id);

    if (CondFlag[imitators])
    {
      if (sq_capture==queenside_castling)
        joueim(+dir_right);
      else if (rochade)
        joueim((3*sq_arrival-sq_departure-rochade_sq[coup_id]) / 2);
      else if (sq_capture!=kingside_castling) /* joueim(0) (do nothing) if OO */
        joueim(sq_arrival-sq_departure);
    }
  }

  spec_pi_captured = pprispec[ply_id] = spec[sq_capture];
  pi_captured = pprise[ply_id] = e[sq_capture];

  if (sq_arrival==nullsquare)
    return true;
  else
  {
    if (anyantimars && sq_departure==sq_capture || move_gen_top->capture >= platzwechsel_rochade)
    {
      spec_pi_captured = 0;
      pprispec[ply_id]= 0;
      pi_captured = vide;
      pprise[ply_id] = vide;
    }

    pdisp[ply_id] = vide;
    pdispspec[ply_id] = 0;

    if (jouegenre)
    {
      if (CondFlag[blsupertrans_king]
          && trait_ply==Black
          && ctrans[coup_id]!=vide)
      {
        king_square[Black]=initsquare;
        pi_arriving=ctrans[coup_id];
      }
      if (CondFlag[whsupertrans_king]
          && trait_ply==White
          && ctrans[coup_id]!=vide)
      {
        king_square[White]=initsquare;
        pi_arriving=ctrans[coup_id];
      }

      if (CondFlag[singlebox] && SingleBoxType==singlebox_type3
          && sb3[coup_id].what!=vide) {
        --nbpiece[e[sb3[coup_id].where]];
        e[sb3[coup_id].where] = sb3[coup_id].what;
        ++nbpiece[sb3[coup_id].what];
        if (sq_departure==sb3[coup_id].where) {
          pi_departing = pi_arriving = sb3[coup_id].what;
        }
      }

      if (CondFlag[ghostchess] && pi_captured!=vide)
        remember_ghost(sq_arrival);
      if (CondFlag[hauntedchess] && pi_captured!=vide)
      {
        preempt_ghost(sq_arrival);
        remember_ghost(sq_arrival);
      }
    }

    if (TSTFLAG(spec_pi_moving, ColourChange)) {
      sq_hurdle= chop[coup_id];
    }

    switch (sq_capture)
    {
      case messigny_exchange:
        pprise[ply_id]= e[sq_departure]= e[sq_arrival];
        pprispec[ply_id]= spec[sq_departure]= spec[sq_arrival];
        jouearr[ply_id]= e[sq_arrival]= pi_departing;
        spec[sq_arrival]= spec_pi_moving;
        if (king_square[White]==sq_departure)
          king_square[White] = sq_arrival;
        else if (king_square[White]==sq_arrival)
          king_square[White] = sq_departure;
        if (king_square[Black]==sq_departure)
          king_square[Black] = sq_arrival;
        else if (king_square[Black]==sq_arrival)
          king_square[Black]= sq_departure;

        return jouecoup_legality_test();

      case kingside_castling:
        if (CondFlag[einstein])
        {
          if (trait_ply==White)
          {
            nbpiece[tb]--;
            if (CondFlag[reveinstein])
            {
              e[square_f1]= db;
              nbpiece[db]++;
            }
            else
            {
              e[square_f1]= fb;
              nbpiece[fb]++;
            }
          }
          else
          {
            nbpiece[tn]--;
            if (CondFlag[reveinstein])
            {
              e[square_f8]= dn;
              nbpiece[dn]++;
            }
            else
            {
              e[square_f8]= fn;
              nbpiece[fn]++;
            }
          }
        }
        else
          e[sq_departure+dir_right]= e[sq_departure+3*dir_right];

        spec[sq_departure+dir_right]= spec[sq_departure+3*dir_right];
        e[sq_departure+3*dir_right]= CondFlag[haanerchess] ? obs : vide;
        CLEARFL(spec[sq_departure+3*dir_right]);
        CLRCASTLINGFLAGMASK(ply_id,trait_ply,k_castling);
        CLRCASTLINGFLAGMASK(ply_id,advers(trait_ply),
                            castling_mutual_exclusive[trait_ply][kingside_castling-min_castling]);
        break;

      case queenside_castling:
        if (CondFlag[einstein])
        {
          if (trait_ply==White)
          {
            nbpiece[tb]--;
            if (CondFlag[reveinstein])
            {
              e[square_d1]= db;
              nbpiece[db]++;
            }
            else
            {
              e[square_d1]= fb;
              nbpiece[fb]++;
            }
          }
          else
          {
            nbpiece[tn]--;
            if (CondFlag[reveinstein])
            {
              e[square_d8]= dn;
              nbpiece[dn]++;
            }
            else
            {
              e[square_d8]= fn;
              nbpiece[fn]++;
            }
          }
        }
        else
          e[sq_departure+dir_left]= e[sq_departure+4*dir_left];

        spec[sq_departure+dir_left]= spec[sq_departure+4*dir_left];
        e[sq_departure+4*dir_left]= CondFlag[haanerchess] ? obs : vide;
        CLEARFL(spec[sq_departure+4*dir_left]);
        CLRCASTLINGFLAGMASK(ply_id,trait_ply,q_castling);
        CLRCASTLINGFLAGMASK(ply_id,advers(trait_ply),
                            castling_mutual_exclusive[trait_ply][queenside_castling-min_castling]);
        break;
    } /* switch (sq_capture) */

    if (platzwechsel)
    {
      piece p=e[sq_arrival];
      Flags sp=spec[sq_arrival];
      e[sq_arrival]=e[sq_departure];
      spec[sq_arrival]=spec[sq_departure];
      if (king_square[Black] == sq_departure)
        king_square[Black]= sq_arrival;
      if (king_square[White] == sq_departure)
        king_square[White]= sq_arrival;
      e[sq_departure]=p;
      spec[sq_departure]=sp;
    }
    else if (rochade)
    {
      square sq_castle= (sq_departure + sq_arrival) / 2;
      e[sq_castle] = e[rochade_sq[coup_id]];
      spec[sq_castle] = spec[rochade_sq[coup_id]];
      e[rochade_sq[coup_id]] = CondFlag[haanerchess] ? obs : vide;
      CLEARFL(spec[rochade_sq[coup_id]]);
      if (king_square[Black] == rochade_sq[coup_id])
        king_square[Black]= sq_castle;
      if (king_square[White] == rochade_sq[coup_id])
        king_square[White]= sq_castle;
      e[sq_departure]= CondFlag[haanerchess] ? obs : vide;
      spec[sq_departure]= 0;
    }
    else
    {
      e[sq_departure]= CondFlag[haanerchess] ? obs : vide;
      spec[sq_departure]= 0;
    }

    if (PatienceB) {
      ply nply;
      e[sq_departure]= obs;
      for (nply= ply_id - 1 ; nply > 1 ; nply--) {
        if (trait[nply] == trait_ply) {
          e[sqdep[nply]]= vide;
        }
      }
    }

    if (change_moving_piece) {
      /* Now pawn-promotion (even into 'kamikaze'-pieces)
         is possible !  NG */
      if (TSTFLAG(spec_pi_moving, Kamikaze)) {
        if (pi_captured != vide) {
          if (!anycirce) {
            spec_pi_moving= 0;
            pi_arriving= vide;
            /* to avoid useless promotions of a vanishing pawn */
          }
        }
      }
      if (CondFlag[linechamchess])
        pi_arriving= linechampiece(pi_arriving, sq_arrival);

      if (CondFlag[chamchess])
        pi_arriving= champiece(pi_arriving);

      if (TSTFLAG(spec_pi_moving, Chameleon))
        pi_arriving= champiece(pi_arriving);

      if (CondFlag[norsk])
        pi_arriving= norskpiece(pi_arriving);

      if (pi_captured != vide
          && ((CondFlag[protean]
               && (!rex_protean_ex || !TSTFLAG(spec_pi_moving, Royal)))
              || TSTFLAG(spec_pi_moving, Protean))) {
        if (pi_departing < vide) {
          pi_arriving = -pi_captured;
          if (pi_arriving == pn)
            pi_arriving = reversepn;
          else if (pi_arriving == reversepn)
            pi_arriving = pn;
        } else {
          pi_arriving = -pi_captured;
          if (pi_arriving == pb)
            pi_arriving = reversepb;
          else if (pi_arriving == reversepb)
            pi_arriving = pb;
        }
      }
    } /* change_moving_piece */

    if (abs(pi_departing) == andergb) {
      square sq= sq_arrival - direction(sq_departure, sq_arrival);

      if (!TSTFLAG(spec[sq], Neutral) && (sq != king_square[White]) && (sq != king_square[Black])) {
        change(sq);
        CHANGECOLOR(spec[sq]);
      }
    } /* andergb */

    ep[ply_id]= ep2[ply_id]= initsquare;
    if (is_pawn(pi_departing)) {
      if (pi_captured==vide) {
        /* ep capture */
        if (CondFlag[phantom]) {
          int col_diff, rank_j;

          col_diff= sq_arrival%onerow - sq_departure%onerow,
            rank_j= sq_arrival/onerow;

          if (rank_j == 11) { /* 4th rank */
            switch (col_diff) {
            case 0:
              if (pi_departing==pb
                  && sq_departure!=sq_arrival+dir_down)
                ep[ply_id]= sq_arrival+dir_down;
              break;

            case -2:
              if (pi_departing==pbb
                  && sq_departure!=sq_arrival+dir_down+dir_left)
                ep[ply_id]= sq_arrival+dir_down+dir_left;
              break;

            case 2:
              if (pi_departing==pbb
                  && sq_departure!=sq_arrival+dir_down+dir_right)
                ep[ply_id]= sq_arrival+dir_down+dir_right;
              break;
            }
          }
          else if (rank_j == 12) { /* 5th rank */
            switch (col_diff) {
            case 0:
              if (pi_departing==pn
                  && sq_departure!=sq_arrival+dir_up)
                ep[ply_id]= sq_arrival+dir_up;
              break;
            case -2:
              if (pi_departing==pbn
                  && sq_departure!=sq_arrival+dir_up+dir_left)
                ep[ply_id]= sq_arrival+dir_up+dir_left;
              break;
            case 2:
              if (pi_departing==pbn
                  && sq_departure!=sq_arrival+dir_up+dir_right)
                ep[ply_id]= sq_arrival+dir_up+dir_right;
              break;
            }
          }
        } /* CondFlag[phantom] */
        else
        {
          square ii = anyantimars ? cmren[coup_id] : sq_departure;
          switch (abs(pi_departing)) {
          case Pawn:
          case ReversePawn:
            switch (abs(ii - sq_arrival)) {
            case 2*onerow: /* ordinary or Einstein double step */
              ep[ply_id]= (ii + sq_arrival) / 2;
              break;
            case 3*onerow: /* Einstein triple step */
              ep[ply_id]= (ii + sq_arrival + sq_arrival) / 3;
              ep2[ply_id]= (ii + ii + sq_arrival) / 3;
              break;
            } /* end switch (abs(ii-sq_arrival)) */
            break;
          case BerolinaPawn:
            if (abs(ii - sq_arrival) > onerow+1) {
              /* It's a double step! */
              ep[ply_id]= (ii + sq_arrival) / 2;
            }
            break;
          }
        }
      }
    }

    prompieces[ply_id] = GetPromotingPieces(sq_departure,
                                        pi_departing,
                                        trait[ply_id],
                                        spec_pi_moving,
                                        sq_arrival,
                                        pi_captured);

    if (prompieces[ply_id] != NULL)
      {
        pi_arriving = norm_prom[ply_id];
        if (pi_arriving==vide)
        {
          if (!CondFlag[noiprom] && Iprom[ply_id])
          {
            ply icount;
            if (inum[ply_id] == maxinum)
              FtlMsg(ManyImitators);
            for (icount = ply_id; icount<=maxply; ++icount)
              ++inum[icount];
            isquare[inum[ply_id]-1] = sq_arrival;
          }
          else
          {
            pi_arriving= (prompieces[ply_id])[vide];

            if (CondFlag[frischauf])
              SETFLAG(spec_pi_moving, FrischAuf);

            if (pi_captured != vide && anyanticirce) {
  #if defined(BETTER_READABLE)
              /* this coding seems to be better redable */
              do {
                sq_rebirth= (*antirenai)(pi_arriving,
                                         spec_pi_moving,
                                         sq_capture,
                                         sq_departure,
                                         advers(trait_ply));
                if (sq_rebirth == sq_departure)
                  break;
                if (LegalAntiCirceMove(sq_rebirth, sq_capture, sq_departure))
                  break;
                pi_arriving= (prompieces[ply_id])[pi_arriving];
              } while (1);
  #endif /*BETTER_READABLE*/

              while (((sq_rebirth= (*antirenai)(ply_id,
                                                pi_arriving,
                                                spec_pi_moving,
                                                sq_capture,
                                                sq_departure,
                                                sq_arrival,
                                                advers(trait_ply)))
                      != sq_departure)
                     && !LegalAntiCirceMove(sq_rebirth,
                                            sq_capture,
                                            sq_departure))
              {
                pi_arriving= (prompieces[ply_id])[pi_arriving];
                if (!pi_arriving && CondFlag[antisuper])
                {
                  super[ply_id]++;
                  pi_arriving= (prompieces[ply_id])[vide];
                }
              }
            }

            norm_prom[ply_id]= pi_arriving;
          }
        }
        else
        {
          if (CondFlag[frischauf])
            SETFLAG(spec_pi_moving, FrischAuf);
        }

        if (!(!CondFlag[noiprom] && Iprom[ply_id])) {
          if (TSTFLAG(spec_pi_moving, Chameleon)
              && is_pawn(pi_departing))
            norm_cham_prom[ply_id]= true;

          if (pi_departing<vide)
            pi_arriving = -pi_arriving;

          /* so also promoted neutral pawns have the correct color and
           * an additional call to setneutre is not required.
           */
          if (norm_cham_prom[ply_id])
            SETFLAG(spec_pi_moving, Chameleon);
        }
        else
          pi_arriving= vide; /* imitator-promotion */
      } /* promotion */

    if (sq_arrival!=sq_capture)
    {
      e[sq_capture] = vide;
      spec[sq_capture] = 0;
    }

    if (pi_captured!=vide)
    {
      nbpiece[pi_captured]--;

      if (sq_capture==king_square[White])
        king_square[White] = initsquare;
      if (sq_capture==king_square[Black])
        king_square[Black] = initsquare;
    }

    if (change_moving_piece)
    {
      if (CondFlag[degradierung]
          && !is_pawn(pi_departing)
          && sq_departure != prev_rn
          && sq_departure != prev_rb
          && (trait_ply == Black
              ? sq_arrival>=square_a7 && sq_arrival<=square_h7
              : sq_arrival>=square_a2 && sq_arrival<=square_h2))
      {
        if (pi_departing < vide)
          pi_arriving= pn;
        else
          pi_arriving= pb;
      }

      if ((CondFlag[tibet]
           && pi_captured != vide
           && pi_arriving != -pi_captured
           && (((trait_ply == Black)
                && (sq_departure != prev_rn))
               || ((trait_ply == White)
                   && CondFlag[dbltibet]
                   && (sq_departure != prev_rb))))
          || (CondFlag[andernach]
              && pi_captured != vide
              && sq_departure != prev_rn
              && sq_departure != prev_rb))
      {
        SETCOLOR(spec_pi_moving, spec_pi_captured);
        if (CondFlag[volage] && !CondFlag[hypervolage])
          CLRFLAG(spec_pi_moving, Volage);

        /* nasty neutral implementation */
        if (pi_arriving < vide)
          pi_arriving= -pi_arriving;

        /* now the piece is white */
        /* has it to be changed? */
        if (TSTFLAG(spec_pi_moving, Black)
            && (!TSTFLAG(spec_pi_moving, White) || neutcoul == Black))
        {
          pi_arriving= -pi_arriving;
        }
      }

      /* this seems to be misplaced -- it's checked every time TLi */
      /* therefore moved to this place ! NG */
      if (CondFlag[antiandernach]
          && pi_captured == vide
          && sq_departure != prev_rn
          && sq_departure != prev_rb )
      {
        /* the following also copes correctly with neutral */
        CLRFLAG(spec_pi_moving, Black);
        CLRFLAG(spec_pi_moving, White);
        CLRFLAG(spec_pi_moving, Neutral);
        if (trait_ply == Black) {
          SETFLAG(spec_pi_moving, White);
          pi_arriving= abs(pi_arriving);
        }
        else {
          SETFLAG(spec_pi_moving, Black);
          pi_arriving= -abs(pi_arriving);
        }
      } /* CondFlag[antiandernach] ... */

      if (CondFlag[champursue]
        && sq_arrival == move_generation_stack[repere[ply_id]].departure
        && sq_departure != prev_rn
        && sq_departure != prev_rb)
      {
        /* the following also copes correctly with neutral */
        CLRFLAG(spec_pi_moving, Black);
        CLRFLAG(spec_pi_moving, White);
        CLRFLAG(spec_pi_moving, Neutral);
        if (trait_ply == Black) {
          SETFLAG(spec_pi_moving, White);
          pi_arriving= abs(pi_arriving);
        }
        else {
          SETFLAG(spec_pi_moving, Black);
          pi_arriving= -abs(pi_arriving);
        }
      } /* CondFlag[antiandernach] ... */

      if ((CondFlag[traitor]
           && trait_ply == Black
           && sq_arrival<=square_h4
           && !TSTFLAG(spec_pi_moving, Neutral))
          || (TSTFLAG(spec_pi_moving, Volage)
              && SquareCol(sq_departure) != SquareCol(sq_arrival))
          || (TSTFLAG(sq_spec[sq_arrival], MagicSq)
              && prev_rn != sq_departure
              && prev_rb != sq_departure))
      {
        CHANGECOLOR(spec_pi_moving);
        if (!CondFlag[hypervolage])
          CLRFLAG(spec_pi_moving, Volage);
        pi_arriving= -pi_arriving;
      }

      if (CondFlag[einstein]
          && !(CondFlag[antieinstein] && pi_captured != vide))
      {
        pi_arriving= (pi_captured==vide) != CondFlag[reveinstein]
          ? dec_einstein(pi_arriving)
          : inc_einstein(pi_arriving);
      }
    } /* change_moving_piece */

    if (sq_departure==prev_rb)
    {
      if (king_square[White]!=initsquare)
        king_square[White] = sq_arrival;
      CLRCASTLINGFLAGMASK(ply_id,White,k_cancastle);
    }
    if (sq_departure==prev_rn)
    {
      if (king_square[Black]!=initsquare)
        king_square[Black] = sq_arrival;
      CLRCASTLINGFLAGMASK(ply_id,Black,k_cancastle);
    }

    /* Needed for castling */
    if (castling_supported)
    {
      /* pieces vacating a1, h1, a8, h8 */
      if (sq_departure == square_h1)
        CLRCASTLINGFLAGMASK(ply_id,White,rh_cancastle);
      else if (sq_departure == square_a1)
        CLRCASTLINGFLAGMASK(ply_id,White,ra_cancastle);
      else if (sq_departure == square_h8)
        CLRCASTLINGFLAGMASK(ply_id,Black,rh_cancastle);
      else if (sq_departure == square_a8)
        CLRCASTLINGFLAGMASK(ply_id,Black,ra_cancastle);

      /* pieces arriving at a1, h1, a8, h8 and possibly capturing a rook */
      if (sq_arrival == square_h1)
        CLRCASTLINGFLAGMASK(ply_id,White,rh_cancastle);
      else if (sq_arrival == square_a1)
        CLRCASTLINGFLAGMASK(ply_id,White,ra_cancastle);
      else if (sq_arrival == square_h8)
        CLRCASTLINGFLAGMASK(ply_id,Black,rh_cancastle);
      else if (sq_arrival == square_a8)
        CLRCASTLINGFLAGMASK(ply_id,Black,ra_cancastle);

      if (CondFlag[losingchess])
      {
        /* r[bn] (and therefore prev_r[bn]) are ==initsquare if kings
         * are not royal
         */
        if (sq_arrival==square_e1)
          CLRCASTLINGFLAGMASK(ply_id,White,k_cancastle);
        else if (sq_arrival==square_e8)
          CLRCASTLINGFLAGMASK(ply_id,Black,k_cancastle);
      }
    }     /* castling_supported */

    if ((CondFlag[andernach] && pi_captured!=vide)
        || (CondFlag[antiandernach] && pi_captured==vide)
        || (CondFlag[champursue] && sq_arrival == move_generation_stack[repere[ply_id]].departure)
        || (CondFlag[norsk])
        || (CondFlag[protean]
            && (pi_captured!=vide || abs(pi_departing)==ReversePawn))
      )
    {
      if (castling_supported) {
        if (abs(pi_arriving) == Rook) {
          if (TSTFLAG(spec_pi_moving, White)) {
            /* new white/neutral rook */
            if (sq_arrival == square_h1)
              SETCASTLINGFLAGMASK(ply_id,White,rh_cancastle);
            else if (sq_arrival == square_a1)
              SETCASTLINGFLAGMASK(ply_id,White,ra_cancastle);
          }
          if (TSTFLAG(spec_pi_moving, Black)) {
            /* new black/neutral rook */
            if (sq_arrival == square_h8)
              SETCASTLINGFLAGMASK(ply_id,Black,rh_cancastle);
            else if (sq_arrival == square_a8)
              SETCASTLINGFLAGMASK(ply_id,Black,ra_cancastle);
          }
        }
      } /* castling_supported */
    } /* andernach || antiandernach ... */

    if (TSTFLAG(spec_pi_moving, HalfNeutral))
    {
      if (TSTFLAG(spec_pi_moving, Neutral))
      {
        CLRFLAG(spec_pi_moving,advers(trait_ply));
        CLRFLAG(spec_pi_moving, Neutral);
        pi_arriving= trait_ply==Black ? -abs(pi_arriving) : abs(pi_arriving);

        if (king_square[Black] == sq_arrival && trait_ply == White)
          king_square[Black]= initsquare;

        if (king_square[White] == sq_arrival && trait_ply == Black)
          king_square[White]= initsquare;
      }
      else if (trait_ply==Black) {
        if (TSTFLAG(spec_pi_moving, Black)) {
          SETFLAG(spec_pi_moving, Neutral);
          SETFLAG(spec_pi_moving, White);
          pi_arriving= abs(pi_arriving);
          if (king_square[Black] == sq_arrival)
            king_square[White] = sq_arrival;
        }
      }
      else if (trait_ply==White) {
        if (TSTFLAG(spec_pi_moving, White)) {
          SETFLAG(spec_pi_moving, Neutral);
          SETFLAG(spec_pi_moving, Black);
          pi_arriving= -abs(pi_arriving);
          if (king_square[White] == sq_arrival)
            king_square[Black] = sq_arrival;
        }
      }
    }

    if (!platzwechsel)
    {
    e[sq_arrival] = pi_arriving;
    spec[sq_arrival] = spec_pi_moving;
    jouearr[ply_id] = pi_arriving;

    if (pi_departing!=pi_arriving)
    {
      nbpiece[pi_departing]--;
      nbpiece[pi_arriving]++;
    }
    }

    if (jouegenre)
    {
      if (pi_captured != vide)
      {
        if (kobulking[Black] && trait_ply == White && king_square[Black] != initsquare)
        {
          PieSpec sp;
          piece kobul = is_pawn(pi_captured) ? roin : pi_captured;
          nbpiece[e[king_square[Black]]]--;
          e[king_square[Black]] = kobul;
          if (TSTFLAG(spec[king_square[Black]], Neutral))
            setneutre(king_square[Black]);
          for (sp = Kamikaze; sp < PieSpCount; sp++)
            if (sp != Royal)
              TSTFLAG(spec_pi_captured, sp) ?
                SETFLAG(spec[king_square[Black]], sp) :
                CLRFLAG(spec[king_square[Black]], sp);
          nbpiece[kobul]++;
        }
        if (kobulking[White] && trait_ply == Black && king_square[White] != initsquare)
        {
          PieSpec sp;
          piece kobul = is_pawn(pi_captured) ? roib : pi_captured;
          nbpiece[e[king_square[White]]]--;
          e[king_square[White]] = kobul;
          if (TSTFLAG(spec[king_square[Black]], Neutral))
            setneutre(king_square[Black]);
          for (sp = Kamikaze; sp < PieSpCount; sp++)
            if (sp != Royal)
              TSTFLAG(spec_pi_captured, sp) ?
                SETFLAG(spec[king_square[White]], sp) :
                CLRFLAG(spec[king_square[White]], sp);
          nbpiece[kobul]++;
        }
      }

      if (CondFlag[ghostchess] || CondFlag[hauntedchess])
        summon_ghost(sq_departure);

      if (TSTFLAG(spec_pi_moving, HalfNeutral)
          && TSTFLAG(spec_pi_moving, Neutral))
        setneutre(sq_arrival);

      /* Duellantenschach */
      if (CondFlag[duellist]) {
        if (trait_ply == Black) {
          whduell[ply_id]= whduell[ply_id - 1];
          blduell[ply_id]= sq_arrival;
        }
        else {
          blduell[ply_id]= blduell[ply_id - 1];
          whduell[ply_id]= sq_arrival;
        }
      }

      if (CondFlag[singlebox] && SingleBoxType==singlebox_type2)
      {
        Side adv = advers(trait_ply);

        if (sb2[ply_id].where==initsquare)
        {
          assert(sb2[ply_id].what==vide);
          sb2[ply_id].where = next_latent_pawn(initsquare,adv);
          if (sb2[ply_id].where!=initsquare)
          {
            sb2[ply_id].what = next_singlebox_prom(vide,adv);
            if (sb2[ply_id].what==vide)
              sb2[ply_id].where = initsquare;
          }
        }

        if (sb2[ply_id].where!=initsquare)
        {
          assert(e[sb2[ply_id].where] == (adv==White ? pb : pn));
          assert(sb2[ply_id].what!=vide);
          --nbpiece[e[sb2[ply_id].where]];
          e[sb2[ply_id].where] = (adv==White
                                  ? sb2[ply_id].what
                                  : -sb2[ply_id].what);
          ++nbpiece[e[sb2[ply_id].where]];
        }
      }

      /* AntiCirce */
      if (pi_captured != vide && anyanticirce) {
        sq_rebirth= (*antirenai)(ply_id,
                                 pi_arriving,
                                 spec_pi_moving,
                                 sq_capture,
                                 sq_departure,
                                 sq_arrival,
                                 advers(trait_ply));
        if (CondFlag[antisuper])
        {
          while (!LegalAntiCirceMove(sq_rebirth, sq_capture, sq_departure))
            sq_rebirth++;
          super[ply_id]= sq_rebirth;
        }
        e[sq_arrival]= vide;
        spec[sq_arrival]= 0;
        sq_rebirth_capturing[ply_id]= sq_rebirth;
        prompieces[ply_id]= GetPromotingPieces(sq_departure,
                                     pi_departing,
                                     trait_ply,
                                     spec_pi_moving,
                                     sq_rebirth,
                                     pi_captured);

        if (prompieces[ply_id])
        {
          /* white pawn on eighth rank or
             black pawn on first rank - promotion ! */
          nbpiece[pi_arriving]--;
          pi_arriving= norm_prom[ply_id];
          if (pi_arriving == vide)
            norm_prom[ply_id]= pi_arriving= prompieces[ply_id][vide];
          if (pi_departing < vide)
            pi_arriving= -pi_arriving;
          nbpiece[pi_arriving]++;
        }
        e[sq_rebirth]= pi_arriving;
        spec[sq_rebirth]= spec_pi_moving;
        if (sq_departure == prev_rb)
          king_square[White]= sq_rebirth;
        if (sq_departure == prev_rn)
          king_square[Black]= sq_rebirth;

        if (castling_supported) {
          PieNam const abspja= abs(pi_arriving);
          if (abspja==King) {
            if (TSTFLAG(spec_pi_moving, White)
                && sq_rebirth == square_e1
                && (!CondFlag[dynasty] || nbpiece[roib]==1))
              /* white king new on e1 */
              SETCASTLINGFLAGMASK(ply_id,White,k_cancastle);
            else if (TSTFLAG(spec_pi_moving, Black)
                     && sq_rebirth == square_e8
                     && (!CondFlag[dynasty] || nbpiece[roin]==1))
              /* black king new on e8 */
              SETCASTLINGFLAGMASK(ply_id,Black,k_cancastle);
          }
          else if (abspja==Rook) {
            if (TSTFLAG(spec_pi_moving, White)) {
              /* new white/neutral rook */
              if (sq_rebirth == square_h1)
                SETCASTLINGFLAGMASK(ply_id,White,rh_cancastle);
              else if (sq_rebirth == square_a1)
                SETCASTLINGFLAGMASK(ply_id,White,ra_cancastle);
            }
            if (TSTFLAG(spec_pi_moving, Black)) {
              /* new black/neutral rook */
              if (sq_rebirth == square_h8)
                SETCASTLINGFLAGMASK(ply_id,Black,rh_cancastle);
              else if (sq_rebirth == square_a8)
                SETCASTLINGFLAGMASK(ply_id,Black,ra_cancastle);
            }
          }
        } /* castling_supported */
      } /* AntiCirce */

      if (flag_outputmultiplecolourchanges)
      {
        colour_change_sp[ply_id] = colour_change_sp[ply_id - 1];
      }

      if (flag_magic)
      {
        PushMagicViews();
        ChangeMagic(ply_id, flag_outputmultiplecolourchanges);
      }

      if (CondFlag[sentinelles])
      {
        if (sq_departure>=square_a2 && sq_departure<=square_h7
            && !is_pawn(pi_departing))
        {
          if (SentPionNeutral) {
            if (TSTFLAG(spec_pi_moving, Neutral)) {
              nbpiece[e[sq_departure]= sentinelb]++;
              SETFLAG(spec[sq_departure], Neutral);
              setneutre(sq_departure);
            }
            else if ((trait_ply==Black) != SentPionAdverse) {
              nbpiece[e[sq_departure]= sentineln]++;
              SETFLAG(spec[sq_departure], Black);
            }
            else {
              nbpiece[e[sq_departure]= sentinelb]++;
              SETFLAG(spec[sq_departure], White);
            }
            if (   nbpiece[sentinelb] > max_pb
                   || nbpiece[sentineln] > max_pn
                   || nbpiece[sentinelb]+nbpiece[sentineln] > max_pt)
            {
              /* rules for sentinelles + neutrals not yet
                 written but it's very likely this logic will
                 need to be refined
              */
              nbpiece[e[sq_departure]]--;
              e[sq_departure]= vide;
              spec[sq_departure]= 0;
            }
            else {
              senti[ply_id]= true;
            }
          }
          else if ((trait_ply==Black) != SentPionAdverse) {
            if (   nbpiece[sentineln] < max_pn
                   && nbpiece[sentinelb]+nbpiece[sentineln]<max_pt
                   && (  !flagparasent
                         ||(nbpiece[sentineln]
                            <= nbpiece[sentinelb]
                            +(pi_captured==sentinelb ? 1 : 0))))
            {
              nbpiece[e[sq_departure]= sentineln]++;
              SETFLAG(spec[sq_departure], Black);
              senti[ply_id]= true;
            }
          }
          else if ( nbpiece[sentinelb] < max_pb
                    && nbpiece[sentinelb]+nbpiece[sentineln]<max_pt
                    && (!flagparasent
                        ||(nbpiece[sentinelb]
                           <= nbpiece[sentineln]
                           + (pi_captured==sentineln ? 1 : 0))))
          {
            nbpiece[e[sq_departure]= sentinelb]++;
            SETFLAG(spec[sq_departure], White);
            senti[ply_id]= true;
          }
        }
      }

      if (anycirce)
      {
        TraceSquare(super[ply_id]);TraceText("\n");
        if (pi_captured!=vide
            && CondFlag[circecage] && super[ply_id]==superbas)
        {
          norm_prom[ply_id] = vide;
          cir_prom[ply_id] = vide;
          circecage_find_initial_cage(ply_id,
                                      pi_departing,sq_arrival,pi_captured,
                                      &super[ply_id],
                                      &cir_prom[ply_id],
                                      &norm_prom[ply_id]);
        }

        /* circe-rebirth of moving kamikaze-piece */
        if (TSTFLAG(spec_pi_moving, Kamikaze) && (pi_captured != vide)) {
          if (CondFlag[couscous]) {
            sq_rebirth= (*circerenai)(ply_id,
                                      pi_captured,
                                      spec_pi_captured,
                                      sq_capture,
                                      sq_departure,
                                      sq_arrival,
                                      trait_ply);
          }
          else {
            sq_rebirth= (*circerenai)(ply_id,
                                      pi_arriving,
                                      spec_pi_moving,
                                      sq_capture,
                                      sq_departure,
                                      sq_arrival,
                                      advers(trait_ply));
          }

          e[sq_arrival]= vide;
          spec[sq_arrival]= 0;
          if ((e[sq_rebirth] == vide)
              && !(CondFlag[contactgrid]
                   && nogridcontact(sq_rebirth)))
          {
            sq_rebirth_capturing[ply_id]= sq_rebirth;
            e[sq_rebirth]= pi_arriving;
            spec[sq_rebirth]= spec_pi_moving;
            if (rex_circe) {
              if (sq_departure == prev_rb)
                king_square[White]= sq_rebirth;
              if (sq_departure == prev_rn)
                king_square[Black]= sq_rebirth;

              if (castling_supported
                  && (abs(pi_arriving) == King)) {
                if (TSTFLAG(spec_pi_moving, White)
                    && sq_rebirth == square_e1
                    && (!CondFlag[dynasty] || nbpiece[roib]==1))
                  /* white king reborn on e1 */
                  SETCASTLINGFLAGMASK(ply_id,White,k_cancastle);
                else if (TSTFLAG(spec_pi_moving, Black)
                         && sq_rebirth == square_e8
                         && (!CondFlag[dynasty] || nbpiece[roin]==1))
                  /* black king reborn on e8 */
                  SETCASTLINGFLAGMASK(ply_id,Black,k_cancastle);
              }
            }
            if (castling_supported
                && (abs(pi_arriving) == Rook)) {
              if (TSTFLAG(spec_pi_moving, White)) {
                if (sq_rebirth == square_h1)
                  /* white rook reborn on h1 */
                  SETCASTLINGFLAGMASK(ply_id,White,rh_cancastle);
                else if (sq_rebirth == square_a1)
                  /* white rook reborn on a1 */
                  SETCASTLINGFLAGMASK(ply_id,White,ra_cancastle);
              }
              if (TSTFLAG(spec_pi_moving, Black)) {
                if (sq_rebirth == square_h8)
                  /* black rook reborn on h8 */
                  SETCASTLINGFLAGMASK(ply_id,Black,rh_cancastle);
                else if (sq_rebirth == square_a8)
                  /* black rook reborn on a8 */
                  SETCASTLINGFLAGMASK(ply_id,Black,ra_cancastle);
              }
            }
          } else
            nbpiece[pi_arriving]--;
        } /* Kamikaze */

        if (anyparrain)
        {
          if (pprise[ply_id-1]!=vide)
            joueparrain(ply_id);
        }
        else
        {
          if (pi_captured != vide) {
            if (anyclone
                && sq_departure != prev_rn && sq_departure != prev_rb) {
              /* Circe Clone - new implementation
              ** captured pieces are reborn as pieces
              ** of the same type as the capturing piece
              ** if the latter one is not royal.
              */
              /* change type of pieces according to colour */
              pi_reborn = ((pi_departing * pi_captured < 0)
                           ? -pi_departing
                           : pi_departing);
              /* If it is a pawn give it the pawn-attribut.
              ** Otherwise delete it - the captured piece may
              ** have been a pawn, but is not anymore.
              */
            }
            else if (anytraitor) {
              pdispspec[ply_id]=spec_pi_captured;
              pi_reborn= -pi_captured;
              CHANGECOLOR(spec_pi_captured);
            }
            else {
              if (CondFlag[chamcirce]) {
                pi_reborn= ChamCircePiece(pi_captured);
              }
              else if (CondFlag[antieinstein]) {
                pi_reborn= inc_einstein(pi_captured);
              }
              else {
                pi_reborn= pi_captured;
              }
            }

            if (CondFlag[couscous])
              sq_rebirth= (*circerenai)(ply_id,
                                        pi_arriving,
                                        spec_pi_moving,
                                        sq_capture,
                                        sq_departure,
                                        sq_arrival,
                                        advers(trait_ply));
            else
              sq_rebirth= (*circerenai)(ply_id,
                                        pi_reborn,
                                        spec_pi_captured,
                                        sq_capture,
                                        sq_departure,
                                        sq_arrival,
                                        trait_ply);

            if (!rex_circe
                && (CondFlag[exclusive] || CondFlag[isardam] || CondFlag[ohneschach] || CondFlag[brunner])
                && (sq_capture == prev_rb || sq_capture == prev_rn))
            {
              /* ordinary circe and (isardam, brunner or
              ** ohneschach) it can happen that the king is
              ** captured while testing the legality of a
              ** move. Therefore prevent the king from being
              ** reborn.
              */
              sq_rebirth= initsquare;
            }

            if ( (e[sq_rebirth] == vide || CondFlag[circeassassin])
                 && !( CondFlag[contactgrid]
                       && nogridcontact(sq_rebirth)))
            {
              sqrenais[ply_id]= sq_rebirth;
              if (rex_circe) {
                /* neutral K */
                if (prev_rb == sq_capture) {
                  king_square[White]= sq_rebirth;
                }
                if (prev_rn == sq_capture) {
                  king_square[Black]= sq_rebirth;
                }

                if (castling_supported
                    && (abs(pi_reborn) == King)) {
                  if (TSTFLAG(spec_pi_captured, White)
                      && sq_rebirth == square_e1
                      && (!CondFlag[dynasty] || nbpiece[roib]==1))
                    /* white king reborn on e1 */
                    SETCASTLINGFLAGMASK(ply_id,White,k_cancastle);
                  else if (TSTFLAG(spec_pi_captured, Black)
                           && sq_rebirth == square_e8
                           && (!CondFlag[dynasty] || nbpiece[roin]==1))
                    /* black king reborn on e8 */
                    SETCASTLINGFLAGMASK(ply_id,Black,k_cancastle);
                }
              }

              if (castling_supported
                  && (abs(pi_reborn) == Rook)) {
                if (TSTFLAG(spec_pi_captured, White)) {
                  if (sq_rebirth == square_h1)
                    /* white rook reborn on h1 */
                    SETCASTLINGFLAGMASK(ply_id,White,rh_cancastle);
                  else if (sq_rebirth == square_a1)
                    /* white rook reborn on a1 */
                    SETCASTLINGFLAGMASK(ply_id,White,ra_cancastle);
                }
                if (TSTFLAG(spec_pi_captured, Black)) {
                  if (sq_rebirth == square_h8)
                    /* black rook reborn on h8 */
                    SETCASTLINGFLAGMASK(ply_id,Black,rh_cancastle);
                  else if (sq_rebirth == square_a8)
                    /* black rook reborn on a8 */
                    SETCASTLINGFLAGMASK(ply_id,Black,ra_cancastle);
                }
              }
              if (anycirprom
                  && is_pawn(pi_captured)
                  && PromSq(advers(trait_ply),sq_rebirth))
              {
                /* captured white pawn on eighth rank: promotion ! */
                /* captured black pawn on first rank: promotion ! */
                piece pprom = cir_prom[ply_id];
                if (pprom==vide)
                {
                  pprom = getprompiece[vide];
                  cir_prom[ply_id] = pprom;
                }
                pi_reborn = pi_reborn<vide ? -pprom : pprom;
                if (cir_cham_prom[ply_id])
                  SETFLAG(spec_pi_captured, Chameleon);
              }
              if (TSTFLAG(spec_pi_captured, Volage)
                  && SquareCol(sq_rebirth) != SquareCol(sq_capture))
              {
                pi_reborn= -pi_reborn;
                CHANGECOLOR(spec_pi_captured);
                if (!CondFlag[hypervolage]) {
                  CLRFLAG(spec_pi_captured, Volage);
                }
              }
              if (CondFlag[circeassassin]) {
                nbpiece[pdisp[ply_id]=e[sq_rebirth]]--;
                pdispspec[ply_id]=spec[sq_rebirth];
              }
              nbpiece[e[sq_rebirth]= pi_reborn]++;
              spec[sq_rebirth]= spec_pi_captured;
            }
          }
        }
      }

      if (bl_royal_sq != initsquare)
        king_square[Black]= bl_royal_sq;

      if (wh_royal_sq != initsquare)
        king_square[White]= wh_royal_sq;

      if (CondFlag[republican])
        republican_place_king(jt,trait_ply,ply_id);

      if (CondFlag[actrevolving])
        transformPosition(rot270);

      if (CondFlag[arc])
      {
        if (sq_departure==square_d4 || sq_departure==square_e4
            || sq_departure==square_d5 || sq_departure==square_e5
            || sq_arrival==square_d4 || sq_arrival==square_e4
            || sq_arrival==square_d5 || sq_arrival==square_e5)
        {
          piece const piece_temp = e[square_d4];
          Flags const spec_temp = spec[square_d4];

          e[square_d4]= e[square_e4];
          spec[square_d4]= spec[square_e4];

          e[square_e4]= e[square_e5];
          spec[square_e4]= spec[square_e5];

          e[square_e5]= e[square_d5];
          spec[square_e5]= spec[square_d5];

          e[square_d5]= piece_temp;
          spec[square_d5]= spec_temp;

          if (king_square[White]==square_d4)
            king_square[White]= square_d5;
          else if (king_square[White]==square_d5)
            king_square[White]= square_e5;
          else if (king_square[White]==square_e5)
            king_square[White]= square_e4;
          else if (king_square[White]==square_e4)
            king_square[White]= square_d4;

          if (king_square[Black]==square_d4)
            king_square[Black]= square_d5;
          else if (king_square[Black]==square_d5)
            king_square[Black]= square_e5;
          else if (king_square[Black]==square_e5)
            king_square[Black]= square_e4;
          else if (king_square[Black]==square_e4)
            king_square[Black]= square_d4;
        }
      }

      /* move to here to make sure it is definitely set through jouecoup
      otherwise repcoup can osc Ks even if not oscillated in jouecoup */
      oscillatedKs[ply_id]= false;
      if (trait_ply==White
          ? CondFlag[white_oscillatingKs]
          : CondFlag[black_oscillatingKs]) {
        boolean priorcheck= false;
        square temp= king_square[White];
        piece temp1= e[king_square[White]];
        Flags temp2= spec[king_square[White]];

        if (OscillatingKingsTypeB[trait_ply])
          priorcheck= echecc(ply_id,trait_ply);
        if ((oscillatedKs[ply_id]= (!OscillatingKingsTypeC[trait_ply]
                                   || echecc(ply_id,advers(trait_ply)))))
        {
          e[king_square[White]]= e[king_square[Black]];
          spec[king_square[White]]= spec[king_square[Black]];

          e[king_square[Black]]= temp1;
          spec[king_square[Black]]= temp2;
          king_square[White]= king_square[Black];
          king_square[Black]= temp;
          CLRCASTLINGFLAGMASK(ply_id,White,k_cancastle);
          CLRCASTLINGFLAGMASK(ply_id,Black,k_cancastle);
          if (king_square[White]==square_e1)
            SETCASTLINGFLAGMASK(ply_id,White,k_cancastle);
          if (king_square[Black]==square_e8)
            SETCASTLINGFLAGMASK(ply_id,Black,k_cancastle);
          if (OscillatingKingsTypeB[trait_ply] && priorcheck)
            return false;
        }
      }

      if (TSTFLAG(spec_pi_moving, ColourChange)) {
        if (abs(pi_hurdle= e[sq_hurdle]) > roib) {
          nbpiece[pi_hurdle]--;
          e[sq_hurdle]= -pi_hurdle;
          nbpiece[-pi_hurdle]++;
          CHANGECOLOR(spec[sq_hurdle]);
        }
      }

      if (CondFlag[dynasty]) {
        /* adjust king_square[Black], king_square[White] and/or castling flags */
        square const *bnp;
        square s;

        if (nbpiece[roib]==1) {
          if (king_square[White]==initsquare)
            for (bnp= boardnum; *bnp; bnp++) {
              s = *bnp;
              if (e[s] == roib) {
                if (s==square_e1)
                  SETCASTLINGFLAGMASK(ply_id,White,k_cancastle);
                king_square[White] = *bnp;
                break;
              }
            }
        }
        else
          king_square[White] = initsquare;

        if (nbpiece[roin]==1) {
          if (king_square[Black]==initsquare)
            for (bnp= boardnum; *bnp; bnp++) {
              s = *bnp;
              if (e[s] == roin) {
                if (s==square_e8)
                  SETCASTLINGFLAGMASK(ply_id,Black,k_cancastle);
                king_square[Black] = *bnp;
                break;
              }
            }
        }
        else
          king_square[Black] = initsquare;
      }

      if (CondFlag[strictSAT] && SATCheck)
      {
        WhiteStrictSAT[ply_id]= (WhiteStrictSAT[parent_ply[ply_id]]
                                 || echecc_normal(ply_id,White));
        BlackStrictSAT[ply_id]= (BlackStrictSAT[parent_ply[ply_id]]
                                 || echecc_normal(ply_id,Black));
      }

      if (CondFlag[masand]
          && echecc(ply_id,advers(trait_ply))
          && observed(trait_ply == White ? king_square[Black] : king_square[White],
                      move_gen_top->arrival))
        change_observed(ply_id,
                        move_gen_top->arrival,
                        flag_outputmultiplecolourchanges);
    } /* if (jouegenre) */

    return jouecoup_legality_test();
  }
} /* end of jouecoup */

void repcoup(void)
{
  square sq_rebirth;
  piece pi_departing, pi_captured, pi_hurdle;
  Flags spec_pi_moving;
  boolean next_prom = true;
  square nextsuper= superbas;
  square sq_hurdle;
  boolean rochade=false;
  boolean platzwechsel = false;

  move_generation_elmt* move_gen_top = move_generation_stack+nbcou;

  square sq_departure= move_gen_top->departure;
  square sq_arrival= move_gen_top->arrival;
  square sq_capture= move_gen_top->capture;

  if (jouegenre)
  {
    if (sq_capture > platzwechsel_rochade)
    {
       sq_capture= sq_arrival;
       rochade= true;
    }
    else if (sq_capture == platzwechsel_rochade)
    {
      sq_capture= sq_arrival;
      platzwechsel= true;
    }


    if (flag_magic)
      ChangeMagic(nbply, false);

    if (CondFlag[masand]
        && echecc(nbply,advers(trait[nbply]))
        && observed(trait[nbply] == White ? king_square[Black] : king_square[White],
                    sq_arrival))
      change_observed(nbply,sq_arrival,false);

    if (oscillatedKs[nbply])  /* for Osc Type C */
    {
      square temp= king_square[White];
      piece temp1= e[king_square[White]];
      Flags temp2= spec[king_square[White]];
      e[king_square[White]]= e[king_square[Black]];
      spec[king_square[White]]= spec[king_square[Black]];
      e[king_square[Black]]= temp1;
      spec[king_square[Black]]= temp2;
      king_square[White]= king_square[Black];
      king_square[Black]= temp;
    }

    if (CondFlag[actrevolving])
      transformPosition(rot90);

    if (CondFlag[arc])
    {
      /* transformPosition(rot90); */
      if (sq_departure==square_d4 || sq_departure==square_e4
          || sq_departure==square_d5 || sq_departure==square_e5
          || sq_arrival==square_d4 || sq_arrival==square_e4
          || sq_arrival==square_d5 || sq_arrival==square_e5)
      {
        Flags temp=spec[square_d4];
        piece ptemp=e[square_d4];
        e[square_d4]= e[square_d5];
        spec[square_d4]= spec[square_d5];
        e[square_d5]= e[square_e5];
        spec[square_d5]= spec[square_e5];
        e[square_e5]= e[square_e4];
        spec[square_e5]= spec[square_e4];
        e[square_e4]= ptemp;
        spec[square_e4]= temp;
        if (king_square[White]==square_d4) {
          king_square[White]= square_e4;
        }
        else if (king_square[White]==square_d5) {
          king_square[White]= square_d4;
        }
        else if (king_square[White]==square_e5) {
          king_square[White]= square_d5;
        }
        else if (king_square[White]==square_e4) {
          king_square[White]= square_e5;
        }
        if (king_square[Black]==square_d4) {
          king_square[Black]= square_e4;
        }
        else if (king_square[Black]==square_d5) {
          king_square[Black]= square_d4;
        }
        else if (king_square[Black]==square_e5) {
          king_square[Black]= square_d5;
        }
        else if (king_square[Black]==square_e4) {
          king_square[Black]= square_e5;
        }
      }
    }

    if (CondFlag[singlebox] && SingleBoxType==singlebox_type2)
    {
      sb2[nbply+1].where = initsquare;
      sb2[nbply+1].what = vide;

      if (sb2[nbply].where!=initsquare)
      {
        Side adv = advers(trait[nbply]);

        assert(sb2[nbply].what!=vide);
        --nbpiece[e[sb2[nbply].where]];
        e[sb2[nbply].where] = adv==White ? pb : pn;
        ++nbpiece[e[sb2[nbply].where]];

        sb2[nbply].what = next_singlebox_prom(sb2[nbply].what,adv);
        if (sb2[nbply].what==vide)
        {
          sb2[nbply].where = next_latent_pawn(sb2[nbply].where,adv);
          if (sb2[nbply].where!=initsquare)
          {
            sb2[nbply].what = next_singlebox_prom(vide,adv);
            assert(sb2[nbply].what!=vide);
          }
        }

        next_prom = sb2[nbply].where==initsquare;
      }
    }
  } /* jouegenre */

  pi_captured= pprise[nbply];
  pi_departing= pjoue[nbply];
  spec_pi_moving= jouespec[nbply];

  if (sq_arrival==nullsquare)
  {
    nbcou--;
    return;
  }

  if (jouegenre)
  {
    if (CondFlag[ghostchess] && pi_captured!=vide)
      forget_ghost(sq_arrival);
    if (CondFlag[hauntedchess] && pi_captured!=vide)
    {
      forget_ghost(sq_arrival);
      unpreempt_ghost(sq_arrival);
    }

    if (CondFlag[singlebox] && SingleBoxType==singlebox_type3
        && sb3[nbcou].what!=vide) {
      piece pawn = trait[nbply]==White ? pb : pn;
      e[sb3[nbcou].where] = pawn;
      if (sq_departure!=sb3[nbcou].where) {
        --nbpiece[sb3[nbcou].what];
        ++nbpiece[pawn];
      }
    }

    if (TSTFLAG(spec_pi_moving, ColourChange)) {
      sq_hurdle= chop[nbcou];
      if (abs(pi_hurdle= e[sq_hurdle]) > roib) {
        nbpiece[pi_hurdle]--;
        e[sq_hurdle]= -pi_hurdle;
        nbpiece[-pi_hurdle]++;
        CHANGECOLOR(spec[sq_hurdle]);
      }
    }
  }

  castling_flag[nbply]= castling_flag[nbply-1];
  whpwr[nbply]= whpwr[nbply-1];
  blpwr[nbply]= blpwr[nbply-1];

  switch (sq_capture) {
  case messigny_exchange:
    e[sq_arrival]= e[sq_departure];
    spec[sq_arrival]= spec[sq_departure];
    e[sq_departure]= pi_departing;
    spec[sq_departure]= spec_pi_moving;
    nbcou--;
    king_square[White]= RB_[nbply];
    king_square[Black]= RN_[nbply];
    return;

  case kingside_castling:
    if (CondFlag[einstein]) {
      if (sq_departure == square_e1) {   /* white */
        e[square_h1]= tb;
        nbpiece[tb]++;
        if (CondFlag[reveinstein])
          nbpiece[db]--;
        else
          nbpiece[fb]--;
      }
      else {    /* black */
        e[square_h8]= tn;
        nbpiece[tn]++;
        if (CondFlag[reveinstein])
          nbpiece[dn]--;
        else
          nbpiece[fn]--;
      }
    }
    else {
      e[sq_departure+3*dir_right]= e[sq_departure+dir_right];
    }
    spec[sq_departure+3*dir_right]= spec[sq_departure+dir_right];
    e[sq_departure+dir_right]= vide;
    CLEARFL(spec[sq_departure+dir_right]);
    /* reset everything */
    break;

  case queenside_castling:
    if (CondFlag[einstein]) {
      if (sq_departure == square_e1) {    /* white */
        e[square_a1]= tb;
        nbpiece[tb]++;
        if (CondFlag[reveinstein])
          nbpiece[db]--;
        else
          nbpiece[fb]--;
      }
      else {     /* black */
        e[square_a8]= tn;
        nbpiece[tn]++;
        if (CondFlag[reveinstein])
          nbpiece[dn]--;
        else
          nbpiece[fn]--;
      }
    }
    else {
      e[sq_departure+4*dir_left]= e[sq_departure+dir_left];
    }
    spec[sq_departure+4*dir_left]= spec[sq_departure+dir_left];
    e[sq_departure+dir_left]= vide;
    CLEARFL(spec[sq_departure+dir_left]);
    /* reset everything */
    break;

  } /* switch (sq_capture) */

  /* the supercirce rebirth square has to be evaluated here in the
  ** position after the capture. Otherwise it is more difficult to
  ** allow the rebirth on the original square of the capturing piece
  ** or in connection with locust or e.p. captures.
  */
  if ((CondFlag[supercirce] && pi_captured!=vide)
      || isapril[abs(pi_captured)]
      || (CondFlag[antisuper] && pi_captured!=vide))
  {
    nextsuper = super[nbply]+1;

    while (e[nextsuper]!=vide && nextsuper<square_h8)
      ++nextsuper;

    if (CondFlag[antisuper]
        && AntiCirCheylan
        && nextsuper==sq_capture)
    {
      ++nextsuper;
      while (e[nextsuper]!=vide && nextsuper<square_h8)
        ++nextsuper;
    }
  }

  if (CondFlag[circecage] && pi_captured!=vide)
  {
    piece circecage_next_norm_prom = norm_prom[nbply];
    piece circecage_next_cage_prom = cir_prom[nbply];
    square const currcage = super[nbply];
    square nextcage = currcage;

    if (circecage_next_cage_prom!=vide)
      circecage_advance_cage_prom(nbply,nextcage,&circecage_next_cage_prom);

    if (circecage_next_cage_prom==vide)
    {
      /* prevent current prisoner from disturbing the search for the
       * next cage
       */
      piece const currprisoner = e[currcage];
      e[currcage] = vide;

      circecage_advance_cage(nbply,
                             pi_captured,
                             &nextcage,
                             &circecage_next_cage_prom);

      if (nextcage>square_h8)
      {
        nextcage = superbas;
        if (circecage_next_norm_prom!=vide)
          circecage_advance_norm_prom(nbply,
                                      sq_arrival,pi_captured,
                                      &nextcage,
                                      &circecage_next_cage_prom,
                                      &circecage_next_norm_prom);
      }

      e[currcage] = currprisoner;
    }

    super[nbply] = nextcage;
    cir_prom[nbply] = circecage_next_cage_prom;
    norm_prom[nbply] = circecage_next_norm_prom;

    if (nextcage!=superbas)
      next_prom = false;
  }

  if (CondFlag[republican])
    republican_unplace_king();

  /* first delete all changes */
  if (repgenre)
  {
    if (trait[nbply] == White && kobulking[Black] && king_square[Black] != initsquare)
    {
      nbpiece[e[king_square[Black]]]--;
      e[king_square[Black]] = blkobul[nbply];
      spec[king_square[Black]] = blkobulspec[nbply];
      nbpiece[blkobul[nbply]]++;
    }
    if (trait[nbply] == Black && kobulking[White] && king_square[White] != initsquare)
    {
      nbpiece[e[king_square[White]]]--;
      e[king_square[White]] = whkobul[nbply];
      spec[king_square[White]] = whkobulspec[nbply];
      nbpiece[whkobul[nbply]]++;
    }

    if ((CondFlag[ghostchess] || CondFlag[hauntedchess])
        && e[sq_departure]!=vide)
      ban_ghost(sq_departure);

    if (senti[nbply]) {
      --nbpiece[e[sq_departure]];
      senti[nbply]= false;
    }
    if (CondFlag[imitators])
    {
      if (sq_capture == queenside_castling)
        joueim(+dir_left);
      else if (rochade)
        joueim((sq_departure + rochade_sq[nbcou] - 3*sq_arrival) / 2);
      else if (sq_capture != kingside_castling) /* joueim(0) (do nothing) if OO */
        joueim(sq_departure - sq_arrival);      /* verschoben TLi */
    }

    if (TSTFLAG(PieSpExFlags, Neutral)) {
      /* the following is faster !  TLi
       * initneutre((pi_departing > vide) ? White : Black);
       */

      if (TSTFLAG(spec_pi_moving, Neutral) &&
          (pi_departing < vide ? Black : White) != neutcoul)
        pi_departing= -pi_departing;
      if (TSTFLAG(pprispec[nbply], Neutral) &&
          (pi_captured < vide ? Black : White) != neutcoul)
        pi_captured= -pi_captured;
    }
    sq_rebirth = sqrenais[nbply];
    if (sq_rebirth!=initsquare)
    {
      sqrenais[nbply] = initsquare;
      if (sq_rebirth != sq_arrival)
      {
        nbpiece[e[sq_rebirth]]--;
        e[sq_rebirth]= vide;
        spec[sq_rebirth]= 0;
      }
      if (CondFlag[circeassassin] && pdisp[nbply]) {
        if (e[sq_rebirth])
          nbpiece[e[sq_rebirth]]--;
        nbpiece[e[sq_rebirth]= pdisp[nbply]]++;
        spec[sq_rebirth]= pdispspec[nbply];
      }
      if (anytraitor)
        spec[sq_rebirth]= pdispspec[nbply];
    }

    if ((sq_rebirth= sq_rebirth_capturing[nbply]) != initsquare) {
      /* Kamikaze and AntiCirce */
      sq_rebirth_capturing[nbply]= initsquare;
      if (sq_rebirth != sq_arrival) {
        nbpiece[e[sq_rebirth]]--;
        e[sq_rebirth]= vide;
        spec[sq_rebirth]= 0;
      }
    }
  } /* if (repgenre) */

  nbpiece[e[sq_arrival]]--;

  /* now reset old position */
  if (sq_arrival != sq_capture) {
    e[sq_arrival]= vide;
    spec[sq_arrival] = 0;
  }
  if (platzwechsel)
  {
    e[sq_arrival]=e[sq_departure];
    spec[sq_arrival]=spec[sq_departure];
  }

  e[sq_departure]= pi_departing;
  spec[sq_departure] = spec_pi_moving;
  nbpiece[pi_departing]++;

  if (rochade)
  {
     square sq_castle= (sq_departure + sq_arrival) / 2;
     e[rochade_sq[nbcou]] = e[sq_castle];
     spec[rochade_sq[nbcou]] = spec[sq_castle];
     e[sq_castle] = vide;
     if (king_square[Black] == sq_castle)
       king_square[Black]= rochade_sq[nbcou];
     if (king_square[White] == sq_castle)
       king_square[White]= rochade_sq[nbcou];
     CLEARFL(spec[sq_castle]);
  }

  if (PatienceB) {
    ply nply;
    for (nply= nbply - 1 ; nply > 1 ; nply--) {
      if (trait[nply] == trait[nbply]) {
        e[sqdep[nply]]= obs;
      }
    }
  }

  if (!platzwechsel && (!anyantimars || sq_capture != sq_departure))
  {
    e[sq_capture]= pi_captured;
    spec[sq_capture]= pprispec[nbply];
  }

  if (pi_captured != vide)
    nbpiece[pi_captured]++;

  king_square[White]= RB_[nbply];
  king_square[Black]= RN_[nbply];

  if (abs(pi_departing) == andergb) {
    square sq= sq_arrival - direction(sq_departure, sq_arrival);

    if (!TSTFLAG(spec[sq], Neutral) && (sq != king_square[White]) && (sq != king_square[Black])) {
      change(sq);
      CHANGECOLOR(spec[sq]);
    }
  } /* andergb */

  /* at last modify promotion-counters and decrement nbcou */
  /* ortho- und pwc-Umwandlung getrennt */
  if (CondFlag[republican])
    next_prom = !republican_advance_king_square();

  if (next_prom)
  {
	piece* prompieceset = prompieces[nbply] ? prompieces[nbply] : getprompiece;

    piece prom_kind_moving = norm_prom[nbply];
    if (prom_kind_moving!=vide)
    {
      prom_kind_moving = prompieceset[prom_kind_moving];
      if (!(CondFlag[singlebox] && SingleBoxType==singlebox_type2))
      {
        if (pi_captured!=vide)
        {
          if (anyanticirce)
            while (prom_kind_moving!=vide)
            {
              sq_rebirth = (*antirenai)(nbply,
                                        prom_kind_moving,
                                        spec_pi_moving,
                                        sq_capture,
                                        sq_departure,
                                        sq_arrival,
                                        advers(trait[nbply]));
              if (sq_rebirth==sq_departure
                  || LegalAntiCirceMove(sq_rebirth,sq_capture,sq_departure))
                break;
              else
                prom_kind_moving = prompieceset[prom_kind_moving];
            }
        }
      }

      norm_prom[nbply] = prom_kind_moving;

      if (prom_kind_moving==vide
          && TSTFLAG(PieSpExFlags,Chameleon)
          && !norm_cham_prom[nbply])
      {
        prom_kind_moving= prompieceset[vide];
        if (pi_captured != vide && anyanticirce)
          while (prom_kind_moving != vide
                 && ((sq_rebirth= (*antirenai)(nbply,
                                               prom_kind_moving,
                                               spec_pi_moving,
                                               sq_capture,
                                               sq_departure,
                                               sq_arrival,
                                               advers(trait[nbply])))
                     != sq_departure)
                 && e[sq_rebirth] != vide)
            prom_kind_moving= prompieceset[prom_kind_moving];

        norm_prom[nbply]= prom_kind_moving;
        norm_cham_prom[nbply]= true;
      }
      if (prom_kind_moving==vide && !CondFlag[noiprom])
        Iprom[nbply]= true;
    }
    else if (!CondFlag[noiprom] && Iprom[nbply])
    {
      ply icount;
      for (icount= nbply; icount<=maxply; icount++)
        --inum[icount];
      Iprom[nbply]= false;
    }

    if (prom_kind_moving == vide)
    {
      piece prom_kind_reborn = vide;
      norm_cham_prom[nbply]= false;
      if (anycirprom)
      {
        prom_kind_reborn = cir_prom[nbply];
        if (prom_kind_reborn!=vide)
        {
          prom_kind_reborn = getprompiece[prom_kind_reborn];
          if (prom_kind_reborn==vide
              && TSTFLAG(PieSpExFlags, Chameleon)
              && !cir_cham_prom[nbply])
          {
            prom_kind_reborn = getprompiece[vide];
            cir_cham_prom[nbply]= true;
          }
        }
      }

      cir_prom[nbply] = prom_kind_reborn;

      if (prom_kind_reborn==vide
          && !(!CondFlag[noiprom] && Iprom[nbply]))
      {
        if ((CondFlag[supercirce] && pi_captured != vide)
                 || isapril[abs(pi_captured)]
                 || (CondFlag[antisuper] && pi_captured != vide))
        {
          super[nbply] = nextsuper;
          if (super[nbply]>square_h8
              || (CondFlag[antisuper]
                  && !LegalAntiCirceMove(nextsuper,sq_capture,sq_departure)))
          {
            super[nbply]= superbas;
            nbcou--;
          }
        }
        else
          nbcou--;
      }
    }
  } /* next_prom*/
} /* end of repcoup */
