function get_files
{
    echo org.kde.step.xml
}

function po_for_file
{
    case "$1" in
       org.kde.step.xml)
           echo step_xml_mimetypes.po
       ;;
    esac
}

function tags_for_file
{
    case "$1" in
       org.kde.step.xml)
           echo comment
       ;;
    esac
}
