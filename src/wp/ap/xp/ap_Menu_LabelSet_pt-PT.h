/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

// We use the Win32 '&' character to denote a keyboard accelerator on a menu item.
// If your platform doesn't have a way to do accelerators or uses a different
// character, remove or change the '&' in your menu constructor code.

// If the third argument is true, then this is the fall-back for
// this language (named in the first argument).

BeginSetEnc(pt,PT,true,"iso-8859-1")

    MenuLabel(AP_MENU_ID__BOGUS1__,		NULL,			NULL)

    //       (id,                           szLabel,            szStatusMsg)

    MenuLabel(AP_MENU_ID_FILE,			"&Ficheiro",		NULL)
    MenuLabel(AP_MENU_ID_FILE_NEW,		"&Novo",		"Criar novo documento")    
    MenuLabel(AP_MENU_ID_FILE_OPEN,		"&Abrir",		"Abrir documento")
    MenuLabel(AP_MENU_ID_FILE_CLOSE,		"Fe&char",		"Fechar documento")
    MenuLabel(AP_MENU_ID_FILE_SAVE,		"&Gravar",		"Gravar documento")
    MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"Gra&var como",		"Gravar documento com novo nome")
    MenuLabel(AP_MENU_ID_FILE_IMPORT,		"&Importar",		"Importar documento para a posição actual do cursor")
    MenuLabel(AP_MENU_ID_FILE_EXPORT,		"&Exportar",		"Gravar cópia do documento com novo nome")

    MenuLabel(AP_MENU_ID_FILE_PAGESETUP,	"&Paginação",		"Modificar Paginação")
    MenuLabel(AP_MENU_ID_FILE_PRINT,		"&Imprimir",		"Imprimir documento")
    MenuLabel(AP_MENU_ID_FILE_PRINT_DIRECTLY,	"Imprimir &Directamente","Imprimir documento com o driver interno de PostScript")
    MenuLabel(AP_MENU_ID_FILE_PRINT_PREVIEW,	"Ver Antes",		"Ver documento antes de imprimir")
    MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",		"Último documento utilizado")
    MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",		"Penúltimo documento utilizado")
    MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",		"Antepenúltimo documento utilizado")
    MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",		"Quarto documento utilizado")
    MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",		"Quinto documento utilizado")
    MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",		"Sexto documento utilizado")
    MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",		"Séptimo documento utilizado")
    MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",		"Oitavo documento utilizado")
    MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",		"Nono documento utilizado")
    MenuLabel(AP_MENU_ID_FILE_REVERT,		"&Reverter",		"Reverter o documento para o último estado em que foi gravado")
    MenuLabel(AP_MENU_ID_FILE_EXIT,		"&Sair",		"Fechar todas as janelas e sair")

    MenuLabel(AP_MENU_ID_EDIT,			"&Editar",		NULL)
    MenuLabel(AP_MENU_ID_EDIT_UNDO,		"&Desfazer",		"Cancelar última acção")
    MenuLabel(AP_MENU_ID_EDIT_REDO,		"&Refazer",		"Refazer última acção cancelada")
    MenuLabel(AP_MENU_ID_EDIT_CUT,		"Co&rtar",		"Cortar selecção")
    MenuLabel(AP_MENU_ID_EDIT_COPY,		"&Copiar",		"Copiar selecção")
    MenuLabel(AP_MENU_ID_EDIT_PASTE,		"Co&lar",		"Colar selecção")
    MenuLabel(AP_MENU_ID_EDIT_PASTE_SPECIAL,	"Colar &Especial",	"Colar objecto não formatado")
    MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"Apa&gar",		"Apagar selecção")
    MenuLabel(AP_MENU_ID_EDIT_SELECTALL,	"Seleccionar &tudo",	"Seleccionar todo o documento")
    MenuLabel(AP_MENU_ID_EDIT_FIND,		"&Procurar",		"Procurar no documento")
    MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"&Substituir",		"Substituir no documento")
    MenuLabel(AP_MENU_ID_EDIT_GOTO,		"&Ir para",		"Deslocar o cursor para")
    MenuLabel(AP_MENU_ID_EDIT_EDITHEADER,	"Editar &Cabeçalho",	"Editar cabeçalho")
    MenuLabel(AP_MENU_ID_EDIT_EDITFOOTER,	"Editar &Rodapé",	"Editar rodapé")
    MenuLabel(AP_MENU_ID_EDIT_REMOVEHEADER,	"Remover Cabeçalho",	"Remover cabeçalho")
    MenuLabel(AP_MENU_ID_EDIT_REMOVEFOOTER,	"Remover Rodapé",	"Remover rodapé")

    MenuLabel(AP_MENU_ID_VIEW,			"&Ver",			NULL)
    MenuLabel(AP_MENU_ID_VIEW_NORMAL,		"Esquema Normal",	"Esquema Normal")
    MenuLabel(AP_MENU_ID_VIEW_WEB,		"Esquema &Web",		"Ver documento como página WWW")
    MenuLabel(AP_MENU_ID_VIEW_PRINT,		"Layout de &Impressão",	"Ver documento como se impresso")
    MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Barras de utilitários", NULL)
    MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Standard",		"Ver/Omitir Barra Standard")
    MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,	"&Formatação",		"Ver/Omitir Barra de Formatação")
    MenuLabel(AP_MENU_ID_VIEW_TB_EXTRA,		"&Extra",		"Ver/Omitir Barra Extra")
    MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Réguas",		"Ver/Omitir Réguas")
    MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,	"Barra de &mensagens",	"Ver/Omitir Barra de Mensagens")
    MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"M&ostrar ¶",		"Ver/Omitir Caracteres Não Imprimíveis")
    MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Cabeçalhos e Rodapés","Editar os cabeçalhos e rodapés")
    MenuLabel(AP_MENU_ID_VIEW_FULLSCREEN,	"&Só a Página",		"Mostrar somente as páginas")
    MenuLabel(AP_MENU_ID_VIEW_ZOOM,		"&Zoom",		"Aproximar ou afastar do texto")
    MenuLabel(AP_MENU_ID_VIEW_ZOOM_MENU,	"&Zoom",		"Aproximar ou afastar do texto")
    MenuLabel(AP_MENU_ID_VIEW_ZOOM_200,		"Zoom to &200%",	"Zoom a 200%")
    MenuLabel(AP_MENU_ID_VIEW_ZOOM_100,		"Zoom to &100%",	"Zoom a 100%")
    MenuLabel(AP_MENU_ID_VIEW_ZOOM_75,		"Zoom to &75%",		"Zoom a 75%")
    MenuLabel(AP_MENU_ID_VIEW_ZOOM_50,		"Zoom to &50%",		"Zoom a 50%")
    MenuLabel(AP_MENU_ID_VIEW_ZOOM_WIDTH,	"Zoom to &Page Width",	"Zoom à largura da página")
    MenuLabel(AP_MENU_ID_VIEW_ZOOM_WHOLE,	"Zoom to &Whole Page",	"Zoom à página inteira")

    MenuLabel(AP_MENU_ID_INSERT,		"&Inserir",		NULL)
    MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Quebra",		"Inserir quebras de página, secção...")
    MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"&Númeração",		"Inserir númeração de páginas")
    MenuLabel(AP_MENU_ID_INSERT_DATETIME,	"Data e &Hora",		"Inserir data e hora")
    MenuLabel(AP_MENU_ID_INSERT_FIELD,		"&Campo",		"Inserir campo calculado")
    MenuLabel(AP_MENU_ID_INSERT_FILE,		"&Inserir Documento",	"Inserir o conteúdo de um outro documento")
    MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"&Símbolo",		"Inserir caracteres especiais")
    MenuLabel(AP_MENU_ID_INSERT_ENDNOTE,	"Nota &Final",		"Inserir nota final")
    MenuLabel(AP_MENU_ID_INSERT_PICTURE,	"&Imagem",		"Inserir imagem")
    MenuLabel(AP_MENU_ID_INSERT_CLIPART,	"&Portfolio",		"Inserir gráficos do portfolio")
#ifdef HAVE_GNOME
    MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,	"Do &Ficheiro",		"Inserir uma imagem a partir de um ficheiro")
#else
    MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,	"&Imagem",		"Inserir imagem")
#endif
    MenuLabel(AP_MENU_ID_INSERT_BOOKMARK,	"&Bookmark",		"Inserir bookmark")
    MenuLabel(AP_MENU_ID_INSERT_HYPERLINK,	"&Hiperligação",	"Inserir hiperligação")

    MenuLabel(AP_MENU_ID_FORMAT,		"F&ormatar",		NULL)
    MenuLabel(AP_MENU_ID_FMT_LANGUAGE,		"&Língua",		"Mudar lígua da área seleccionada")
    MenuLabel(AP_MENU_ID_FMT_FONT,		"&Tipo",		"Mudar tipo de letra")
    MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"&Parágrafo",		"Mudar definição do parágrafo")
    MenuLabel(AP_MENU_ID_FMT_BULLETS,		"&Listas",		"Formatar listas (não) numeradas")
    MenuLabel(AP_MENU_ID_FMT_DOCUMENT,		"&Documento",		"Formatar propriedades do documento, tais como tamanho do papel e margens")
    MenuLabel(AP_MENU_ID_FMT_BORDERS,		"Bordas e &Sombras",	"ajustar bordas e sombreados")
    MenuLabel(AP_MENU_ID_FMT_HDRFTR,		"Formatar Cabeçalho/Rodapé",	"Definir os tipos de Cabeçalhos e Rodapés")
    MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"C&olunas",		"Mudar o número de colunas")
    MenuLabel(AP_MENU_ID_FMT_TOGGLECASE,	"&Maiúsculas/Minúsculas","Mudar capitalização da área seleccionada")
    MenuLabel(AP_MENU_ID_FMT_BACKGROUND,	"&Cor de Fundo",	"Mudar cor de fundo")
    MenuLabel(AP_MENU_ID_FMT_STYLE,		"&Estilo",		"Definir ou aplicar estilos")
    MenuLabel(AP_MENU_ID_FMT_TABS,		"Ta&bulação",		"Definir tabulação")
    MenuLabel(AP_MENU_ID_FMT_BOLD,		"&Negrito",		"Engrossar texto")
    MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Itálico",		"Inclinar texto")
    MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"S&ublinhado",		"Sublinhar texto")
    MenuLabel(AP_MENU_ID_FMT_OVERLINE,		"Sobrelin&hado",	"Sobrelinhar texto")
    MenuLabel(AP_MENU_ID_FMT_STRIKE,		"&Riscar",		"Riscar texto")
    MenuLabel(AP_MENU_ID_FMT_TOPLINE,		"Linha &Superior",	"Colocar/Remover linha acima da selecção")
    MenuLabel(AP_MENU_ID_FMT_BOTTOMLINE,	"Linha &Inferior",	"Colocar/Remover linha abaixo da selecção")
    MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,	"&Expoente",		"Elevar texto")
    MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,		"Ín&dice",		"Indiciar texto")

    MenuLabel(AP_MENU_ID_TOOLS,			"Fe&rramentas",		NULL)
    MenuLabel(AP_MENU_ID_TOOLS_SPELLING,	"&Ortografia",		NULL)
    MenuLabel(AP_MENU_ID_TOOLS_SPELL,		"Verificar agora",	"Verificar ortografia do documento agora")
    MenuLabel(AP_MENU_ID_TOOLS_AUTOSPELL,	"Verificação &Automática","Verificar ortografia do documento automaticamente")
    MenuLabel(AP_MENU_ID_TOOLS_SPELLPREFS,	"&Opções de Ortografia", "Definir opções de ortografia")
    MenuLabel(AP_MENU_ID_TOOLS_WORDCOUNT,	"&Contar palavras",	"Contar palavras no documento")
    MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,		"&Opções",		"Configurar as opções")
    MenuLabel(AP_MENU_ID_TOOLS_LANGUAGE,	"&Língua",		"Mudar a língua da área seleccionada")
    MenuLabel(AP_MENU_ID_TOOLS_PLUGINS,		"P&lugins",		"Gerir plugins")
    MenuLabel(AP_MENU_ID_TOOLS_SCRIPTS,		"S&cripts",		"Executar scripts")
    
    MenuLabel(AP_MENU_ID_ALIGN,			"&Alinhamento",		NULL)
    MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"à &Esquerda",		"Alinhar à esquerda")
    MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Centrar",		"Centrar texto")
    MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"à &Direita",		"Alinhar à direita")
    MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Justificar",		"Justificar texto")

    MenuLabel(AP_MENU_ID_WEB,			"We&b",			NULL)
    MenuLabel(AP_MENU_ID_WEB_SAVEASWEB,		"Gravar como &Web",	"Gravar o documento como uma página de WWW")
    MenuLabel(AP_MENU_ID_WEB_WEBPREVIEW,	"Ver antes como &Web",	"Ver o documento (antes de imprimir) como uma página de WWW")

    MenuLabel(AP_MENU_ID_WINDOW,		"&Janela",		NULL)
    MenuLabel(AP_MENU_ID_WINDOW_NEW,		"&Nova",		"Abrir outra janela para este documento")
    MenuLabel(AP_MENU_ID_WINDOW_1,		"&1 %s",		"Mudar para a primeira janela")
    MenuLabel(AP_MENU_ID_WINDOW_2,		"&2 %s",		"Mudar para a segunda janela")
    MenuLabel(AP_MENU_ID_WINDOW_3,		"&3 %s",		"Mudar para a terceira janela")
    MenuLabel(AP_MENU_ID_WINDOW_4,		"&4 %s",		"Mudar para a quarta janela")
    MenuLabel(AP_MENU_ID_WINDOW_5,		"&5 %s",		"Mudar para a quinta janela")
    MenuLabel(AP_MENU_ID_WINDOW_6,		"&6 %s",		"Mudar para a sexta janela")
    MenuLabel(AP_MENU_ID_WINDOW_7,		"&7 %s",		"Mudar para a septima janela")
    MenuLabel(AP_MENU_ID_WINDOW_8,		"&8 %s",		"Mudar para a oitava janela")
    MenuLabel(AP_MENU_ID_WINDOW_9,		"&9 %s",		"Mudar para a nona janela")
    MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&Mais janelas",	"Mostrar lista completa de janelas")

    MenuLabel(AP_MENU_ID_HELP,			"&Ajuda",		NULL)
    MenuLabel(AP_MENU_ID_HELP_CREDITS,		"C&réditos",		"Mostrar créditos")
    MenuLabel(AP_MENU_ID_HELP_CONTENTS,		"&Tópicos de Ajuda",	"Mostrar tópicos de ajuda")
    MenuLabel(AP_MENU_ID_HELP_INDEX,		"&Index da Ajuda",	"Mostrar o Index da ajuda")
    MenuLabel(AP_MENU_ID_HELP_CHECKVER,		"&Versão",		"Mostrar versão do AbiWord")
    MenuLabel(AP_MENU_ID_HELP_SEARCH,		"&Pesquisar na Ajuda",	"Pesquisar ajuda sobre...")
    MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&Acerca do %s",	"Ver informações sobre o programa, número de versão e copyrights")
    MenuLabel(AP_MENU_ID_HELP_ABOUTOS,		"Acerca de &Open Source",	"Mostrar informação a respeito de Open Source")
    MenuLabel(AP_MENU_ID_HELP_ABOUT_GNU,	"Acerca do Software Livre &GNU", "Saiba mais sobre o Software Livre e o Projecto GNU")
    MenuLabel(AP_MENU_ID_HELP_ABOUT_GNOMEOFFICE, "Acerca do G&NOME Office", "Saiba mais sobre o projecto GNOME Office")

    MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,	"%s",			"Mudar para a sugestão")
    MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,	"%s",			"Mudar para a sugestão")
    MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,	"%s",			"Mudar para a sugestão")
    MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,	"%s",			"Mudar para a sugestão")
    MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,	"%s",			"Mudar para a sugestão")
    MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,	"%s",			"Mudar para a sugestão")
    MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,	"%s",			"Mudar para a sugestão")
    MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,	"%s",			"Mudar para a sugestão")
    MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,	"%s",			"Mudar para a sugestão")
    MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,	"&Ignorar tudo",	"Ignorar todas as ocorrências desta palavra no documento")
    MenuLabel(AP_MENU_ID_SPELL_ADD,		"&Adicionar",		"Adicionar esta palavra ao dicionário custumizável")

    /* autotext submenu labels */
    MenuLabel(AP_MENU_ID_INSERT_AUTOTEXT,	"&Texto Automático",	"")
    MenuLabel(AP_MENU_ID_AUTOTEXT_ATTN,		"Atenção:",		"")
    MenuLabel(AP_MENU_ID_AUTOTEXT_CLOSING,	"Finalizando:",		"")
    MenuLabel(AP_MENU_ID_AUTOTEXT_MAIL,		"Instruções de Correio:","")
    MenuLabel(AP_MENU_ID_AUTOTEXT_REFERENCE,	"Referência:",		"")
    MenuLabel(AP_MENU_ID_AUTOTEXT_SALUTATION,	"Saudações:",		"")
    MenuLabel(AP_MENU_ID_AUTOTEXT_SUBJECT,	"Assunto:",		"")
    MenuLabel(AP_MENU_ID_AUTOTEXT_EMAIL,	"Email:",		"")
    
    //  add others here 

    MenuLabel(AP_MENU_ID__BOGUS2__,		NULL,			NULL)

EndSet()
