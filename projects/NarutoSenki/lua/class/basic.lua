--[[--
	Basic Types
]]

Team = {
    Akatsuki    = 'Akatsuki',
    Konoha      = 'Konoha'
}

local _None = 'None'

ns.CharactersLayout = {
    -- Page One
    -- {
        'Naruto',		'Sakura',		'Sai',			'Kakashi', --[[ Right ]] _None,		    _None,	 	    _None,
        'Shikamaru',	'Ino', 			'Choji',		'Asuma',   --[[ Right ]] 'Kiba', 	    'Hinata', 	    'Shino',
        'Neji', 		'Tenten', 		'Lee', 			'Guy',     --[[ Right ]] _None,         _None, 	        _None,
    -- },
    -- Page Two
    -- {
        'Hashirama', 		    'Tobirama',		'Hiruzen', 		'Minato',   --[[ Right ]] 'Jiraiya', 	'Tsunade', 		'Orochimaru',
        'Gaara',		'Temari',	 		'Kankuro',	 	'Chiyo',    --[[ Right ]] _None,        _None,          _None,
        'Sasuke', 		'Karin', 		'Suigetsu', 	'Jugo',     --[[ Right ]]_None,    _None,          _None,
    -- },
    -- Page Three
    -- {
        _None, 		   'Tobi', 		    'Zetsu',		_None,      --[[ Right ]] 'Pain',        'Konan',         _None,
        _None, 		    'Deidara', 		'Kakuzu', 		'Hidan',    --[[ Right ]] 'Kimimaro',    _None,         _None,
        'Itachi', 		'Kisame',		_None, 		    _None,      --[[ Right ]] _None, 		 _None,         'Obito',
        -- },
 --   Page Four
    -- {
        'Yagura',			'Utakata',			_None,			_None,      --[[ Right ]] _None,		_None,		    _None,
         _None,			_None,			_None,			_None,      --[[ Right ]] _None,		_None,		    _None,
         _None,			_None,			_None,			_None,      --[[ Right ]] _None,		_None,		    _None,
    -- },
  --   Page Four
    -- {
        'Haku',			'Zabuza',			_None,			'Pakura',      --[[ Right ]] _None,		_None,		    _None,
         _None,			_None,			_None,			_None,      --[[ Right ]] _None,		_None,		    _None,
         _None,			_None,			_None,			_None,      --[[ Right ]] _None,		_None,		    _None,
    -- },
}

Heroes.checkLayout(ns.CharactersLayout)
