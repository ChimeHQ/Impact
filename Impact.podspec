Pod::Spec.new do |s|
  s.name         = 'Impact'
  s.version      = '0.3.5'
  s.summary      = 'crash capturing library for Apple platforms'

  s.homepage     = 'https://github.com/stacksift/Impact'
  s.license      = { :type => 'BSD-3-Clause', :file => 'LICENSE' }
  s.author       = { 'Stacksift' => 'support@stacksift.io' }
  s.social_media_url = 'https://twitter.com/stacksift'
  
  s.source        = { :git => 'https://github.com/stacksift/Impact.git', :tag => s.version }

  s.module_name   = "Impact"
  s.source_files  = 'Impact/**/*.{c,h,mm,m}'
  s.public_header_files = 'Impact/Impact.h', 'Impact/ImpactMonitor.h', 'Impact/Monitoring/ImpactMonitoredApplication.h'
  
  s.compiler_flags = '-DCURRENT_PROJECT_VERSION=9', '-I Impact', '-I Impact/Utility', '-I Impact/DWARF', '-I Impact/Unwind', '-I Impact/Monitoring'

  s.libraries = 'c++abi'

  s.osx.deployment_target = '10.13'
  s.ios.deployment_target = '12.0'
  s.tvos.deployment_target = '12.0'

  s.cocoapods_version = '>= 1.4.0'
  s.swift_version = '5.0'
end
