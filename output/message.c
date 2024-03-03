#include "output/plaintext/message.h"
#include "output/plaintext/language_dependant.h"
#include "input/plaintext/language.h"
#include "debugging/assert.h"

static char const *MessageTabs[LanguageCount][MsgCount] =
{
  /*0*/ {
    /* 0*/  "erreur fatale:\n",
    /* 1*/  "il faut un roi de chaque couleur",
    /* 2*/  "Trop peu memoires centrales disponible",
    /* 3*/  "solution interrompue.",
    /* 4*/  "trop de solutions",
    /* 5*/  "en EchecsHaan les pieces kamikazees sont interdites",
    /* 6*/  "jeu d'essai pas applicable",
    /* 7*/  "trop d'echecs feeriques pour les pieces neutres",
    /* 8*/  "leofamily avec pieces orthodoxes ?",
    /* 9*/  "en circe les pieces impuissantes sont interdites",
    /*10*/  "CouleurEchangeantSautoir, Bul et Dob seulement avec quelques sauteurs",
    /*11*/  "cavalier + cavalier majeur interdit",
    /*12*/  "renaicirce hors echiquier ?",
    /*13*/  "jeu apparent avec roi en echec ?",
    /*14*/  "pieces kamikazees + neutres ou pieces kamikazees + circe + pieces feeriques interdites.",
    /*15*/  "solution partielle",
    /*16*/  "on ne peut pas melanger minimum/maximum/duelliste/alphabetique et priseforce pour le meme camp",
    /*17*/  "\nle camp au trait peut prendre le roi",
    /*18*/  "on ne peut pas melanger monochromatique et bichromatique",
    /*19*/  "charactere inconnue: `%s'",
    /*20*/  "plus d'un roi interdit",
    /*21*/  "liste des cases mal terminee - ignoree",
    /*22*/  "liste des cases inconnue ou il mangue une liste des cases - veuillez donner une",
    /*23*/  "numenorute/jusquaucoup hors limites: %s",
    /*24*/  "madrasi, eiffel, isardam, woozles, pieces paralysantes incompatibles avec certains genres feeriques",
    /*25*/  "en rexinclusive, on ne peut pas melanger circe et immunschach",
    /*26*/  "certaines pieces feeriques incompatibles avec heffalumps",
    /*27*/  "enonce avec trops des coups",
    /*28*/  "pas des pieces royales en circe echange ou rex inclusiv.",
    /*29*/  "message inconnue %s",
    /*30*/  "element d'offenser: %s",
    /*31*/  "erreur d'entree:",
    /*32*/  "piece inconnue ou il manque une piece - ignoree",
    /*33*/  "specification de piece pas unique",
    /*34*/  "Echecs Annan et conditions qui changent la coleur de pieces interdit",
    /*35*/  "condition inconnue",
    /*36*/  "option pas unique - ignoree",
    /*37*/  "numero incorrecte",
    /*38*/  "option inconnue - ignoree",
    /*39*/  "specification de commande pas unique - ignoree",
    /*40*/  "commande non connue",
    /*41*/  "pas de specification de l'enonce",
    /*42*/  "Ne peut pas ouvrir fichier de sortie",
    /*43*/  "Ne peut pas ouvrir fichier d'entree",
    /*44*/  "Un trop grand nombre ont emboite des niveaux d'entree",
    /*45*/  "Aucune couleur n'a indique - toute l'entree de cahier des charges ignoree",
    /*46*/  "enonce inconnue",
    /*47*/  "condition pas unique - ignoree",
    /*48*/  "fin de fichier avant fin de probleme",
    /*49*/  "Debordement de ligne d'entree",
    /*50*/  "pas debut probleme",
    /*51*/  "Erreur interne de popeye dans le commutateur de tk",
    /*52*/  "\nsolution terminee.",
    /*53*/  " blocus.",
    /*54*/  " menace:",
    /*55*/  "mais",
    /*56*/  "Temps = ",
    /*57*/  "on mate en un coup par\n",
    /*58*/  "\n",
    /*59*/  "pas de pieces ou conditions trop feeriques avec imitator",
    /*60*/  "trop d'imitators",
    /*61*/  "roi kamikaze seulement en circe rex inclusiv",
    /*62*/  "Gain_de_Piece et Circe Parrain interdit",
    /*63*/  "probleme ignoree",
    /*64*/  "anticirce incompatible avec certains genres feeriques",
    /*65*/  "echecs einstein incompatible avec des pieces feeriques",
    /*66*/  "supercirce incompatible avec certains genres feeriques",
    /*67*/  "entree dans 'hashtable': %s, hit-rate:",
    /*68*/  "On ne peut pas melanger EchecsChameleon et des pieces chameleon",
    /*69*/  "Calculation de",
    /*70*/  "-coups.",
    /*71*/  "enonce inconnue.\n",
    /*72*/  "Multiple buts pas acceptables si l'un d'eux est dia ou a=>b\n",
    /*73*/  "plus de 2 cases pour cle en passant interdit.\n",
    /*74*/  "Termination par signal %d apres ",
    /*75*/  "pieces royales transmutees/reflectees interdites.\n",
    /*76*/  "Rotation/Miroir impossible.",
    /*77*/  "\nUne piece est deplacee hors de l'echiquier.",
    /*78*/  "Enplus doit immediatement suivre jumeau.\n",
    /*79*/  "Refute.",
    /*80*/  "FrischAufCirce: piece n'est pas une piece promue.\n",
    /*81*/  "certaines conditions feeriques et plus courte jeux justificative interdit.\n",
    /*82*/  "Isardam et Madrasi/Eiffel interdit",
    /*83*/  " on ne peut pas determiner si ce coup est legal.\n",
    /*84*/  " deja occupee - piece ancienne va dans l'au-dela.\n",
    /*85*/  "MarsCirce, EchecsPlus, EchecsPhantom ou Make&Take incompatibles avec certains genres feeriques et avec des pieces feeriques.\n",
    /*86*/  "%lu positions possibles dans %u+%u",
    /*87*/  "Melange interdit",
    /*88*/  "On ne peut pas melanger EchecsVogtlander avec Isardam ou EchecsBrunner\n",
    /*89*/  "CirceAssassin est incompatible avec certains genres feeriques et avec des pieces feeriques.\n",
    /*90*/  "enonce doit etre mat avec condition EchecsRepublicain\n",
    /*91*/  ", increment HashRateLevel a %s\n",
    /*92*/  ", decroissance HashRateLevel a %s\n",
    /*93*/  "mode intelligent seulement avec h#/=, ser-#/= et ser-h#/=, facultativement avec certaines conditions, et sans pieces feeriques.\n",
    /*94*/  ": la case est vide - on ne peut pas oter ou deplacer une piece.\n",
    /*95*/  "L'option  MaxTemps  n'est pas soutenu.\n",
    /*96*/  "L'option  FinApresSolutionsCourtes  n'est pas soutenu au jeu direct.\n",
    /*97*/  "Les pieces feeriques ne sont pas permises dans SingleBox",
    /*98*/  "erreur dans FichierLaTeX : piece feerique inconnue\n",
    /*99*/  "Plus de %s types de chasseurs differents\n",
    /*100*/ "Cases et/ou pieces royales incompatibles\n",
    /*101*/ "QuiPerdGagne incompatible avec les enonces echec ou mat\n",
    /*102*/ "Take&MakeEchecs ne sont pas compatibles avec certains genres feeriques\n",
    /*103*/ "Pieces magiques ne sont pas compatibles avec certaines pieces feeriques\n",
    /*104*/ "Trop de calculations magiques necessaires\n",
    /*105*/ "EchecsRepublicains incompatible avec des conditions qui dependent du fait si un coup donne echec.\n",
    /*106*/ "Popeye ne peut pas decider qui a le trait.\n",
    /*107*/ "Option \"menace\" pas applicable ou longueur maximale de la menace trop grande\n",
    /*108*/ "EchecsGhost et EchecsHantes incompatibles avec Circe, Anticirce, Kamikaze, Echecs de Haan\n",
    /*109*/ "ApparentSeul pas applicable - ignore\n",
    /*110*/ "Apparent pas applicable - ignore\n",
    /*111*/ "ApresCle pas applicable - ignore\n",
    /*112*/ "Quodlibet pas applicable - ignore\n",
    /*113*/ "ButEstFin pas applicable - ignore\n",
    /*114*/ "ButEstFin et Quodlibet incompatibles\n",
    /*115*/ "Il ne peut pas etre decide si une condition s'applique aux Blancs ou aux Noirs\n",
    /*116*/ "ImmunCage seulement avec CirceCage\n",
    /*117*/ "On ne peut pas decider si ce coup est refute\n",
    /*118*/ "Capture de roi detectee - elements feeriques probablement incompatibles\n",
    /*119*/ "Information retro inconsistent\n",
    /*120*/ "Options duplex inconsistentes\n",
    /*121*/ "Inconsistent buts pcpj\n",
    /*122*/ "Regles inconsitentents concernant les CoupsVides\n",
    /*123*/ "Elements feeriques incompatibles changeant la marche de pieces\n"
  },
  /*1*/ {
    /* 0*/  "Fataler Fehler:\n",
    /* 1*/  "Es fehlt ein weisser oder schwarzer Koenig\n",
    /* 2*/  "Zu wenig Hauptspeicher verfuegbar\n",
    /* 3*/  "Bearbeitung abgebrochen.",
    /* 4*/  "Zuviele Loesungen",
    /* 5*/  "Im HaanerSchach sind Kamikaze-Steine nicht erlaubt",
    /* 6*/  "Verfuehrung nicht anwendbar\n",
    /* 7*/  "Zu viel Maerchenschach fuer neutrale Steine",
    /* 8*/  "Leofamily mit orthodoxen Figuren nicht erlaubt",
    /* 9*/  "Dummy-Figuren bei Circe nicht erlaubt.",
    /*10*/  "SprungbockFarbeWechselnd, Bul und Dob nur bei einigen Huepfern erlaubt",
    /*11*/  "Springer und Cavalier Majeur nicht erlaubt",
    /*12*/  "Circe-Wiedergeburt ausserhalb des Brettes ?",
    /*13*/  "Satzspiel mit Koenig im Schach ?",
    /*14*/  "Kamikaze + Neutrale Steine oder Kamikaze + Circe + Maerchenfiguren nicht gleichzeitig erlaubt",
    /*15*/  "Partielle Loesung",
    /*16*/  "Laengst-/Kuerzestzueger/Schlagzwang/Duellantenschach/AlphabetischesSchach gleichzeitig fuer die gleiche Partei ?",
    /*17*/  "\nDie am Zug befindliche Partei kann den Koenig schlagen",
    /*18*/  "Monochromes und Bichromes Schach gleichzeitig ergibt keinen Sinn",
    /*19*/  "Unzulaessiges Zeichen: `%s'",
    /*20*/  "Nur ein Koenig erlaubt",
    /*21*/  "Falsch beendete Feldliste - ignoriert",
    /*22*/  "Falsche oder fehlende Feldliste - bitte eine eingeben",
    /*23*/  "Start/BisZugnummer ausserhalb der Grenzen: %s",
    /*24*/  "Eiffelschach, Madrasi, Isardam, Woozles oder paralysierende Steine unvereinbar mit manchen Maerchenbedingungen",
    /*25*/  "Circe und Immunschach und RexInklusive nicht erlaubt",
    /*26*/  "Manche Maerchenfiguren mit Heffalumps unvereinbar",
    /*27*/  "Zuviele Zuege in der Forderung",
    /*28*/  "Keine koeniglichen Figuren im PlatzwechselCirce oder RexInklusive\n",
    /*29*/  "Undefinierter Fehler %s",
    /*30*/  "Stoerende Eingabe: %s",
    /*31*/  "Eingabe-Fehler:",
    /*32*/  "Falsche oder fehlende Steinbezeichnung - wird ignoriert",
    /*33*/  "Steinbezeichnung nicht eindeutig",
    /*34*/  "AnnanSchach und Steine umfaerbende Bedingungen nicht erlaubt",
    /*35*/  "Bedingung nicht erkannt",
    /*36*/  "Option nicht eindeutig - ignoriert",
    /*37*/  "Keine oder falsche Zahlenangabe",
    /*38*/  "Option nicht erkannt - Einlesen von Optionen beendet",
    /*39*/  "Kommando nicht eindeutig - wird ignoriert",
    /*40*/  "Kommando unbekannt",
    /*41*/  "Keine Forderung angegeben",
    /*42*/  "Datei kann nicht zum Schreiben geoeffnet werden",
    /*43*/  "Datei kann nicht zum Lesen geoeffnet werden",
    /*44*/  "Zu viele geschachtelte Eingabedateien",
    /*45*/  "Keine Farbe angegeben - alle Spezifikationen ignoriert",
    /*46*/  "Forderung nicht bekannt",
    /*47*/  "Bedingung nicht eindeutig - ignoriert",
    /*48*/  "Ende der Eingabe vor EndeProblem",
    /*49*/  "Eingabezeilen Ueberlauf",
    /*50*/  "Kein AnfangProblem",
    /*51*/  "Interner Popeye Fehler in tk switch",
    /*52*/  "\nLoesung beendet.",
    /*53*/  " Zugzwang.",
    /*54*/  " Drohung:",
    /*55*/  "Aber",
    /*56*/  "Zeit = ",
    /*57*/  "Matt in einem Zug durch\n",
    /*58*/  "\n",
    /*59*/  "Zuviel Maerchenschach fuer Imitator",
    /*60*/  "Zuviele Imitatoren",
    /*61*/  "Kamikaze-Koenige nur in Circe RexInklusive",
    /*62*/  "Steingewinn Aufgaben mit Circe Parrain sind Frevel",
    /*63*/  "Problem uebersprungen",
    /*64*/  "AntiCirce mit einigen anderen Bedingungen nicht erlaubt",
    /*65*/  "EinsteinSchach nicht mit Maerchenfiguren erlaubt",
    /*66*/  "SuperCirce unvertraeglich mit einigen Maerchenbedingungen",
    /*67*/  "Stellungen: %s, Trefferrate:",
    /*68*/  "Chamaeleonschach und Chamaeleonsteine nicht gleichzeitig erlaubt",
    /*69*/  "Pruefe als",
    /*70*/  "-Zueger.",
    /*71*/  "Forderung wird nicht unterstuetzt.\n",
    /*72*/  "Mehrere Ziele nicht erlaubt, wenn eines davon dia oder a=>b ist\n",
    /*73*/  "Es koennen maximal zwei Felder fuer ep-Schluessel eingegeben werden.\n",
    /*74*/  "Abbruch durch Signal %d nach ",
    /*75*/  "transmutierende/reflektierende koenigliche Steine nicht erlaubt.\n",
    /*76*/  "Drehung/Spiegelung nicht erlaubt: ",
    /*77*/  "\nEin Stein wird ueber den Brettrand verschoben.",
    /*78*/  "Ferner muss als erstes nach Zwilling angegeben werden.\n",
    /*79*/  "Widerlegt.",
    /*80*/  "FrischAufCirce: Stein ist keine Umwandlungsfigur, gg. Steine zuerst eingeben.\n",
    /*81*/  "Manche Maerchenbedingungen in Beweispartien nicht erlaubt.\n",
    /*82*/  "Isardam und Madrasi/Eiffelschach nicht erlaubt",
    /*83*/  " kann nicht entscheiden, ob dieser Zug legal ist.\n",
    /*84*/  " ist bereits besetzt - alter Stein geht in die Unterwelt.\n",
    /*85*/  "MarsCirce, PlusSchach, PhantomSchach oder Make&Take unvereinbar mit verschiedenen Maerchenbedingungen und -steinen.\n",
    /*86*/  "%lu moegliche Stellungen in %u+%u",
    /*87*/  "Unsinnige Kombination",
    /*88*/  "VogtlanderSchach nicht erlaubt in Verbindung mit Isardam oder BrunnerSchach\n",
    /*89*/  "Circe Assassin unvereinbar mit verschiedenen Maerchenbedingungen und -steinen\n",
    /*90*/  "Nur Mattforderung im RepublikanerSchach erlaubt\n",
    /*91*/  ", erhoehe HashRateLevel auf %s\n",
    /*92*/  ", vermindere HashRateLevel auf %s\n",
    /*93*/  "Intelligent Modus nur im h#/=, ser-#/= und ser-h#/=, wenigen Maerchenbedingungen und keinen Maerchensteinen.\n",
    /*94*/  ": Feld leer - kann keine Figur entfernen/versetzen.\n",
    /*95*/  "Option  MaxZeit  nicht unterstuetzt.\n",
    /*96*/  "Option  StopNachKurzloesungen  nicht unterstuetzt im direkten Spiel.\n",
    /*97*/  "Maerchensteine nicht erlaubt in Single box",
    /*98*/  "LaTeXdatei nicht uebersetzbar: Maerchenstein nicht spezifiziert.\n",
    /*99*/  "Mehr als %s verschiedene Jaegertypen\n",
    /*100*/ "Unvereinbare koenigliche Felder und/oder Steine\n",
    /*101*/ "Schlagschach unvereinbar mit Schach- oder Mattforderungen\n",
    /*102*/ "Take&MakeSchach unvereinbar mit verschiedenen Maerchenbedingungen\n",
    /*103*/ "Magische Steine unvereinbar mit verschiedenen Maerchenfiguren\n",
    /*104*/ "Zu viele magische Berechnungen benoetigt\n",
    /*105*/ "Republikanerschach nicht mit Bedingungen, die davon abhangen, ob gewisse Zuege Schach bieten.\n",
    /*106*/ "Es ist nicht feststellbar, wer am Zug ist\n",
    /*107*/ "Option \"Drohung\" nicht anwendbar order maximale Laenge der Drohung zu gross\n",
    /*108*/ "Geisterschach und Spukschach unvereinbar mit Circe, Anticirce, Kamikaze, Haaner Schach\n",
    /*109*/ "WeissBeginnt nicht anwendbar - ignoriert\n",
    /*110*/ "Satzspiel nicht anwendbar - ignoriert\n",
    /*111*/ "NachSchluessel nicht anwendbar - ignoriert\n",
    /*112*/ "Quodlibet nicht anwendbar - ignoriert\n",
    /*113*/ "ZielIstEnde nicht anwendbar - ignoriert\n",
    /*114*/ "ZielIstEnde und Quodlibet nicht kompatibel\n",
    /*115*/ "Es ist nicht entscheidbar, ob Bedingung fuer Weiss oder Schwarz gilt\n",
    /*116*/ "KaefigImmunSchach nur mit KaefigCirce\n",
    /*117*/ "Kann nicht entscheiden, ob dieser Zug widerlegt wird\n",
    /*118*/ "Schlag eines Koenigs detektiert - Maerchenelemente scheinen unvereinbar\n",
    /*119*/ "Inkonsistente Retro-Information\n",
    /*120*/ "Inkonsistente Duplex-Optionen\n",
    /*121*/ "Inkonsistente Beweispartie-Ziele\n",
    /*122*/ "Inkonsistente Regeln betreffend NullZuegen\n",
    /*123*/ "Inkompatible Maerchenelemente, welche die Gangart aendern\n"
  },
  /*2*/ {
    /* 0*/  "fatal error:\n",
    /* 1*/  "both sides need a king",
    /* 2*/  "too less main memory available",
    /* 3*/  "solving interrupted.",
    /* 4*/  "too many solutions",
    /* 5*/  "haaner chess not allowed with kamikaze-pieces",
    /* 6*/  "try play not applicable\n",
    /* 7*/  "too much fairy chess for neutral pieces",
    /* 8*/  "leofamily with orthodox pieces ?",
    /* 9*/  "dummy-pieces not allowed with circe.",
    /*10*/  "HurdleColourChanging, Bul and Dob only allowed with certain hoppers",
    /*11*/  "knight + cavalier majeur not allowed",
    /*12*/  "circe rebirth outside the board ?",
    /*13*/  "set play with checked king ?",
    /*14*/  "kamikaze pieces + neutral pieces or kamikaze pieces + circe + fairy pieces not allowed",
    /*15*/  "Partial solution",
    /*16*/  "maxi-/minimummer/mustcapture/duellistchess/alphabeticchess at the same time for the same side not allowed",
    /*17*/  "\nthe side to play can capture the king",
    /*18*/  "monochrom + bichrom chess gives no sense",
    /*19*/  "wrong character: `%s'",
    /*20*/  "only one king allowed",
    /*21*/  "wrong terminated SquareList - ignored",
    /*22*/  "wrong or missing SquareList - please give one",
    /*23*/  "start/uptomovenumber out of range: %s",
    /*24*/  "eiffel, madrasi, isardam, woozles or paralysing pieces incompatible with some fairy conditions",
    /*25*/  "circe and immunschach and rex inclusiv not allowed",
    /*26*/  "some fairy-pieces not compatible with heffalumps",
    /*27*/  "stipulation with too many moves",
    /*28*/  "no royal pieces with PWC or rex inclusive",
    /*29*/  "undefined Message %s",
    /*30*/  "offending item: %s",
    /*31*/  "input-error:",
    /*32*/  "wrong or missing PieceName - ignored",
    /*33*/  "PieceSpecification not uniq",
    /*34*/  "Annan Chess and conditions that recolour pieces not allowed",
    /*35*/  "could not recognize condition",
    /*36*/  "option not uniq - ignored",
    /*37*/  "wrong integer",
    /*38*/  "could not recognize option - parsing of options aborted",
    /*39*/  "command not uniq specified - ignored",
    /*40*/  "command not known",
    /*41*/  "no stipulation specified",
    /*42*/  "cannot open output-file",
    /*43*/  "cannot open input-file",
    /*44*/  "too many nested input levels",
    /*45*/  "no color specified - all specification input ignored",
    /*46*/  "cannot recognize the stipulation",
    /*47*/  "condition not uniq - ignored",
    /*48*/  "end of file before end of problem",
    /*49*/  "input line overflow",
    /*50*/  "no begin of problem",
    /*51*/  "internal popeye error in tk switch",
    /*52*/  "\nsolution finished.",
    /*53*/  " zugzwang.",
    /*54*/  " threat:",
    /*55*/  "but",
    /*56*/  "Time = ",
    /*57*/  "mate in one move with\n",
    /*58*/  "\n",
    /*59*/  "conditions too fairy for imitator",
    /*60*/  "too many imitators",
    /*61*/  "kamikaze king only in Circe rex inclusiv",
    /*62*/  "Win_a_Piece and Circe Parrain not allowed",
    /*63*/  "problem ignored",
    /*64*/  "anticirce incompatible with some fairy conditions",
    /*65*/  "einsteinchess not allowed with fairy pieces",
    /*66*/  "supercirce incompatible with some fairy conditions",
    /*67*/  "positions: %s, hitrate:",
    /*68*/  "ChameleonChess and ChameleonPieces not allowed together",
    /*69*/  "Checking in",
    /*70*/  "_moves.",
    /*71*/  "Stipulation not supported.\n",
    /*72*/  "Multiple goals not allowed if one of them is dia or a=>b\n",
    /*73*/  "Not more than 2 squares can be putin for en passant keys.\n",
    /*74*/  "Abort by signal %d after ",
    /*75*/  "Transmuting/reflecting royal pieces not supported.\n",
    /*76*/  "This rotation/mirroring is not supported: ",
    /*77*/  "\nA piece will be moved outside.",
    /*78*/  "Continued must be given immediately after twin.\n",
    /*79*/  "refutes.",
    /*80*/  "FrischAufCirce: piece is not a promoted piece. Enter the pieces first.\n",
    /*81*/  "Some fairy conditions in proof games not allowed.\n",
    /*82*/  "Isardam and Madrasi/Eiffel not allowed",
    /*83*/  " cannot decide if this move is legal or not.\n",
    /*84*/  " already occupied - old piece goes to the underworld.\n",
    /*85*/  "MarsCirce, PlusChess, PhantomChess or Make&Take are incompatible with several fairy conditions and fairy pieces.\n",
    /*86*/  "%lu potential positions in %u+%u",
    /*87*/  "nonsense combination",
    /*88*/  "VogtlanderChess not allowed in combination with Isardam or BrunnerChess\n",
    /*89*/  "Circe Assassin is incompatible with several fairy conditions and fairy pieces\n",
    /*90*/  "Must stipulate mate with Republican Chess\n",
    /*91*/  ", increment HashRateLevel to %s\n",
    /*92*/  ", decrement HashRateLevel to %s\n",
    /*93*/  "Intelligent Mode only with h#/=, ser-#/= and ser-h#/=, with a limited set of fairy conditions and without fairy pieces.\n",
    /*94*/  ": square is empty - cannot (re)move any piece.\n",
    /*95*/  "Option  MaxTime  not supported.\n",
    /*96*/  "Option  StopOnShortSolutions  not supported with direct play.\n",
    /*97*/  "Fairy pieces not allowed in Single box",
    /*98*/  "LaTeXFile will not compile: undefined fairy piece\n",
    /*99*/  "Limit of %s different hunter types exceeded\n",
    /*100*/ "Incompatible royal pieces and/or squares\n",
    /*101*/ "LosingChess incompatible with check or mate stipulations\n",
    /*102*/ "Take&MakeChess is incompatible with some fairy conditions\n",
    /*103*/ "Magic pieces incompatible with some fairy pieces\n",
    /*104*/ "Too many magic calculations needed\n",
    /*105*/ "RepublicanChess incompatible with conditions that depend on whether a certain move delivers check.\n",
    /*106*/ "Can't decide which side is at the move\n",
    /*107*/ "Option \"Threat\" not applicable or indicated maximum threat length too big\n",
    /*108*/ "GhostChess and HauntedChess incompatible with Circe, Anticirce, Kamikaze, Haaner Chess\n",
    /*109*/ "WhiteToPlay not applicable - ignored\n",
    /*110*/ "SetPlay not applicable - ignored\n",
    /*111*/ "PostKeyPlay not applicable - ignored\n",
    /*112*/ "Quodlibet not applicable - ignored\n",
    /*113*/ "GoalIsEnd not applicable - ignored\n",
    /*114*/ "GoalIsEnd and Quodlibet are incompatible\n",
    /*115*/ "Can't decide whether condition applies to White or Black\n",
    /*116*/ "CageImmunChess only with CageCirce\n",
    /*117*/ "Can't decide whether this move is refuted\n",
    /*118*/ "King capture detected - fairy elements seem incompatible",
    /*119*/ "Inconsistent retro information\n",
    /*120*/ "Inconsistent duplex options\n",
    /*121*/ "Inconsistent proof game targets\n",
    /*122*/ "Inconsistent rules regarding NullMoves\n",
    /*123*/ "Incompatible fairy elements changing piece types\n"
  }
};

static char const * const *ActualMsgTab = MessageTabs[LanguageDefault];

/* This is used to record an argument which is used
 * in a message-string. There are only message strings
 * that contain at most one format specifier.  Therefore
 * one pointer is sufficient.
 * Three small routines are provided to assign a value:
 */

/* Initialise message for a language
 * @param language the language
 */
void output_message_initialise_language(Language language)
{
  ActualMsgTab = MessageTabs[language];
}

/* Retrieve the message for a specific id in the current language
 * @param id identifies the message to be retrieved
 * @return the message
 */
char const *output_message_get(message_id_t id)
{
  assert(id<MsgCount);
  return ActualMsgTab[id];
}
