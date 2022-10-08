// Følgjande funksjonar er for å få rette artiklar og/eller norske ord på funksjonane i Step.
function bytUt (str)
{
  switch(str)
  {
    case "Partikkel":
      return "ein partikkel";
    case "Ladd partikkel":
      return "ein ladd partikkel";
    case "Plate":
      return "ei plate";
    case "Boks":
      return "boks";
    case "Mangekant":
      return "ein mangekant";
    case "Fjør":
      return "ei fjør";
    case "Lineær motor":
      return "ein lineær motor";
    case "Sirkulær motor":
      return "ein sirkulær motor";
    case "Gass":
      return "gass";
    case "Fleksibel lekam":
      return "ein fleksibel lekam";
    case "Vektkraft":
      return "ei vektkraft";
    case "Tyngdekraft":
      return "ei tyngdekraft";
    case "Coulombkraft":
      return "ei coulombkraft";
    case "Anker":
      return "eit anker";
    case "Plugg":
      return "ein plugg";
    case "Stav":
      return "ein stav";
    case "Merknad":
      return "ein merknad";
    case "Målar":
      return "ein målar";
    case "Spor":
      return "eit spor";
    case "Graf":
      return "ein graf";
    case "Kontrollar":
      return "ein kontrollar";
    case "Particle":
      return "ein partikkel";
    case "ChargedParticle":
      return "ein ladd partikkel";
    case "Disk":
      return "ei plate";
    case "Box":
      return "boks";
    case "Polygon":
      return "ein mangekant";
    case "Spring":
      return "ei fjør";
    case "LinearMotor":
      return "ein lineær motor";
    case "CircularMotor":
      return "ein sirkulær motor";
    case "Gas":
      return "gass";
    case "SoftBody":
      return "ein fleksibel lekam";
    case "WeightForce":
      return "ei vektkraft";
    case "GravitationForce":
      return "ei tyngdekraft";
    case "CoulombForce":
      return "ei coulombkraft";
    case "Anchor":
      return "eit anker";
    case "Pin":
      return "ein plugg";
    case "Stick":
      return "ein stav";
    case "Note":
      return "ein merknad";
    case "Meter":
      return "ein målar";
    case "Tracer":
      return "eit spor";
    case "Graph":
      return "ein graf";
    case "Controller":
      return "ein kontrollar";
    default:
      return str;
  }
}

Ts.setcall("bytUt", bytUt);
