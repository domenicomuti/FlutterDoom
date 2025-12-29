Pod::Spec.new do |s|
  s.name = 'DoomSource'
  s.version          = '1.10.0'
  s.summary          = 'Doom Source Code'
  s.description      = 'Adapted Doom source code for compilation with Xcode'
  s.homepage         = 'hhttps://github.com/domenicomuti/flutter-doom'
  s.license          = { :file => 'linuxdoom-1.10/LICENSE.TXT' }
  s.author           = { 'Domenico Muti' => 'domenico.muti@gmail.com' }

  s.xcconfig         = {
    'OTHER_CFLAGS' => '$(inherited) -DNORMALUNIX -DLINUX -lm'
  }

  s.prepare_command = "chmod +x ../doomsource_apple_workaround.sh && ../doomsource_apple_workaround.sh"
  s.source = { :path => '.' }
  s.source_files = 'linuxdoom-1.10/**/*.{c,m}'
  s.dependency 'Flutter'
  s.platform = :ios, '13.0'
end